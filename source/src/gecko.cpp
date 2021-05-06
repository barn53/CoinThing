#include "gecko.h"
#include "http_json.h"
#include "utils.h"
#include <ArduinoJson.h>

// https://arduinojson.org/v6/assistant/
#define DYNAMIC_JSON_PING_SIZE 64
#define DYNAMIC_JSON_VALID_SIZE 256
#define DYNAMIC_JSON_PRICE_CHANGE_SIZE 256
#define DYNAMIC_JSON_COIN_DETAILS_SIZE 3072
#define DYNAMIC_JSON_CHART_SIZE 3072

#define PRICE_FETCH_INTERVAL (10 * 1000)
#define CHART_48_H_FETCH_INTERVAL (15 * 60 * 1000)
#define CHART_60_D_FETCH_INTERVAL (6 * 60 * 60 * 1000)
#define PING_INTERVAL (2 * 1000)

Gecko::Gecko(HttpJson& http, Settings& settings)
    : m_http(http)
    , m_settings(settings)
{
    m_chart_48h.reserve(48);
    m_chart_60d.reserve(60);
}

void Gecko::begin()
{
    ping();
}

void Gecko::loop()
{
    if (m_succeeded) {
        if (m_settings.valid()) {
            if (m_settings.lastChange() != m_last_seen_settings) {
                m_last_seen_settings = m_settings.lastChange();
                m_last_price_fetch = 0;
                m_last_chart_48h_fetch = 0;
                m_last_chart_60d_fetch = 0;
            }
        } else {
            m_last_price_fetch = 0;
            m_last_seen_settings = 0;
        }
    } else {
        ping();
        m_last_price_fetch = 0;
        m_last_seen_settings = 0;
    }
}

void Gecko::price(gecko_t& price, gecko_t& price2, gecko_t& change_pct)
{
    LOG_FUNC

    if (doInterval(m_last_price_fetch, PRICE_FETCH_INTERVAL)) {
        if (fetchCoinPriceChange()) {
            m_last_price_fetch = millis_test();
            price = m_price;
            price2 = m_price2;
            change_pct = m_change_pct;
        } else {
            m_last_price_fetch = 0;
        }
    }
}

const std::vector<gecko_t>& Gecko::chart_48h(bool& refetched)
{
    LOG_FUNC

    refetched = false;
    if (doInterval(m_last_chart_48h_fetch, CHART_48_H_FETCH_INTERVAL)) {
        if (fetchCoinChart(Settings::ChartPeriod::PERIOD_48_H)) {
            m_last_chart_48h_fetch = millis_test();
            refetched = true;
        } else {
            m_last_chart_48h_fetch = 0;
        }
    }
    return m_chart_48h;
}

const std::vector<gecko_t>& Gecko::chart_60d(bool& refetched)
{
    LOG_FUNC

    refetched = false;
    if (doInterval(m_last_chart_60d_fetch, CHART_60_D_FETCH_INTERVAL)) {
        if (fetchCoinChart(Settings::ChartPeriod::PERIOD_60_D)) {
            m_last_chart_60d_fetch = millis_test();
            refetched = true;
        } else {
            m_last_chart_60d_fetch = 0;
        }
    }
    return m_chart_60d;
}

bool Gecko::fetchCoinPriceChange()
{
    LOG_FUNC

    const char* coin(m_settings.coin());
    const char* currency(m_settings.currency());
    const char* currency2(m_settings.currency2());
    String change24h(currency);
    change24h += F("_24h_change");

    String url(F("https://api.coingecko.com/api/v3/simple/price?ids="));
    url += coin;
    url += F("&vs_currencies=");
    url += currency;
    url += ",";
    url += currency2;
    url += F("&include_24hr_change=true");

    DynamicJsonDocument doc(DYNAMIC_JSON_PRICE_CHANGE_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        m_price = doc[coin][currency] | std::numeric_limits<gecko_t>::infinity();
        m_price2 = doc[coin][currency2] | std::numeric_limits<gecko_t>::infinity();
        m_change_pct = doc[coin][change24h] | std::numeric_limits<gecko_t>::infinity();
        m_last_price_fetch = millis_test();
        return m_price != std::numeric_limits<gecko_t>::infinity()
            && m_change_pct != std::numeric_limits<gecko_t>::infinity();
    }
    return false;
}

bool Gecko::fetchCoinChart(Settings::ChartPeriod type)
{
    LOG_FUNC

#if COIN_THING_SERIAL > 0
    Serial.printf("type: %u\n", type);
#endif

    String url(F("https://api.coingecko.com/api/v3/coins/"));
    url += m_settings.coin();
    url += F("/market_chart?vs_currency=");
    url += m_settings.currency();
    std::vector<gecko_t>* targetChart;

    uint8_t expectValues;
    if (type == Settings::ChartPeriod::PERIOD_24_H
        || type == Settings::ChartPeriod::PERIOD_48_H) {
        url += F("&days=2");
        expectValues = 48;
        m_last_chart_48h_fetch = millis_test();
        targetChart = &m_chart_48h;
    } else if (type == Settings::ChartPeriod::PERIOD_30_D
        || type == Settings::ChartPeriod::PERIOD_60_D) {
        url += F("&days=60&interval=daily");
        expectValues = 60;
        m_last_chart_60d_fetch = millis_test();
        targetChart = &m_chart_60d;
    } else {
        return false;
    }
    targetChart->clear();
    targetChart->reserve(expectValues);

    DynamicJsonDocument filter(16);
    filter["prices"] = true;

    DynamicJsonDocument doc(DYNAMIC_JSON_CHART_SIZE);

    if (m_http.read(url.c_str(), doc, filter)) {
        JsonArray jPrices = doc["prices"];
        size_t jj(jPrices.size());
        for (const auto& p : jPrices) {
            if (jj <= expectValues) { // omit oldest, because there are some more (1, 2) entries in JSON result than expected
                auto f(p[1].as<gecko_t>());
                targetChart->emplace_back(f);
            }
            --jj;
        }
        return !targetChart->empty();
    }
    return false;
}

bool Gecko::fetchCoinDetails(const char* coin, String& symbolInto, String& nameInto) const
{
    LOG_FUNC

    String url(F("https://api.coingecko.com/api/v3/coins/"));
    url += coin;
    url += F("?localization=false&tickers=false&market_data=false&community_data=false&developer_data=false&sparkline=false");

    DynamicJsonDocument filter(32);
    filter["symbol"] = true;
    filter["name"] = true;

    // "sentiment_votes_up_percentage" - todo sentiment lamp

    DynamicJsonDocument doc(DYNAMIC_JSON_COIN_DETAILS_SIZE);

    if (m_http.read(url.c_str(), doc, filter)) {
        symbolInto = doc["symbol"] | "";
        nameInto = doc["name"] | "";
        symbolInto.toUpperCase();
        return true;
    }
    return false;
}

bool Gecko::fetchIsValidCoin(const char* coin) const
{
    LOG_FUNC

    String url(F("https://api.coingecko.com/api/v3/simple/price?ids="));
    url += coin;
    url += F("&vs_currencies=usd");

    DynamicJsonDocument doc(DYNAMIC_JSON_VALID_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        auto gecko_says = doc[coin]["usd"] | std::numeric_limits<gecko_t>::max();
        return gecko_says != std::numeric_limits<gecko_t>::max();
    }
    return false;
}

bool Gecko::ping()
{
    if (doInterval(m_last_ping, PING_INTERVAL)) {
        m_succeeded = false;
        DynamicJsonDocument doc(DYNAMIC_JSON_PING_SIZE);
        if (m_http.read("https://api.coingecko.com/api/v3/ping", doc)) {
            const char* gecko_says = doc["gecko_says"] | ""; // "(V3) To the Moon!"
            m_succeeded = strcmp(gecko_says, "(V3) To the Moon!") == 0;
        }
        m_last_ping = millis_test();
    }
    return m_succeeded;
}

bool Gecko::coinDetails(const char* coinOrSymbol, String& coinInto, String& symbolInto, String& nameInto) const
{
    LOG_FUNC

    String upperCoinOrSymbol(coinOrSymbol);
    upperCoinOrSymbol.toUpperCase();

    for (const auto& c : coins) {
        if (strcmp(c.id, coinOrSymbol) == 0
            || strcmp(c.symbol, upperCoinOrSymbol.c_str()) == 0) {
            coinInto = c.id;
            nameInto = c.name;
            symbolInto = c.symbol;
            return true;
        }
    }
    if (fetchCoinDetails(coinOrSymbol, symbolInto, nameInto)) {
        coinInto = coinOrSymbol;
        return true;
    }
    return false;
}

bool Gecko::isValidCoin(const char* coinOrSymbol) const
{
    LOG_FUNC

    String upperCoinOrSymbol(coinOrSymbol);
    upperCoinOrSymbol.toUpperCase();

    for (const auto& c : coins) {
        if (strcmp(c.id, coinOrSymbol) == 0
            || strcmp(c.symbol, upperCoinOrSymbol.c_str()) == 0) {
            return true;
        }
    }
    return fetchIsValidCoin(coinOrSymbol);
}

bool Gecko::isValidCurrency(const char* currency) const
{
    return isCurrency(currency);
    // no API fallback for currencies
}
