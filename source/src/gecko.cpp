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
#define CHART_30_D_FETCH_INTERVAL (6 * 60 * 60 * 1000)

Gecko::Gecko(HttpJson& http, Settings& settings)
    : m_http(http)
    , m_settings(settings)
{
}

void Gecko::loop()
{
    if (doChange(m_settings.lastChange(), m_last_seen_settings)) {
        m_last_seen_settings = millis_test();
        m_last_price_fetch = 0;
        m_last_chart_48h_fetch = 0;
        m_last_chart_60d_fetch = 0;
    }

    if (doInterval(m_last_price_fetch, PRICE_FETCH_INTERVAL)) {
        if (fetchCoinPriceChange()) {
            m_last_price_fetch = millis_test();
        } else {
            m_last_price_fetch = 0;
        }
    }
}

const std::vector<gecko_t>& Gecko::chart_48h()
{
    if (doInterval(m_last_chart_48h_fetch, CHART_48_H_FETCH_INTERVAL)) {
        if (fetchCoinChart(Settings::Chart::CHART_48_H)) {
            m_last_chart_48h_fetch = millis_test();
        } else {
            m_last_chart_48h_fetch = 0;
        }
    }
    return m_chart_48h;
}

const std::vector<gecko_t>& Gecko::chart_60d()
{
    if (doInterval(m_last_chart_60d_fetch, CHART_30_D_FETCH_INTERVAL)) {
        if (fetchCoinChart(Settings::Chart::CHART_30_D)) {
            m_last_chart_60d_fetch = millis_test();
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
    String change24h(currency);
    change24h += "_24h_change";

    String url("https://api.coingecko.com/api/v3/simple/price?ids=");
    url += coin;
    url += "&vs_currencies=usd,";
    url += currency;
    url += "&include_24hr_change=true";

    DynamicJsonDocument doc(DYNAMIC_JSON_PRICE_CHANGE_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        m_price = doc[coin][currency] | std::numeric_limits<gecko_t>::infinity();
        m_price_usd = doc[coin]["usd"] | std::numeric_limits<gecko_t>::infinity();
        m_change_pct = doc[coin][change24h] | std::numeric_limits<gecko_t>::infinity();
        m_last_price_fetch = millis_test();
        return m_price != std::numeric_limits<gecko_t>::infinity()
            && m_change_pct != std::numeric_limits<gecko_t>::infinity();
    }
    return false;
}

bool Gecko::fetchCoinChart(Settings::Chart type)
{
    LOG_FUNC
#if COIN_THING_SERIAL == 1
    Serial.printf("type: %u\n", type);
#endif

    String url("https://api.coingecko.com/api/v3/coins/");
    url += m_settings.coin();
    url += "/market_chart?vs_currency=";
    url += m_settings.currency();
    std::vector<gecko_t>* targetChart;

    if (type == Settings::Chart::CHART_24_H
        || type == Settings::Chart::CHART_48_H) {
        url += "&days=2";
        m_last_chart_48h_fetch = millis_test();
        targetChart = &m_chart_48h;
    } else if (type == Settings::Chart::CHART_30_D
        || type == Settings::Chart::CHART_60_D) {
        url += "&days=60&interval=daily";
        m_last_chart_60d_fetch = millis_test();
        targetChart = &m_chart_60d;
    } else {
        return false;
    }
    targetChart->clear();

    DynamicJsonDocument filter(16);
    filter["prices"] = true;

    DynamicJsonDocument doc(DYNAMIC_JSON_CHART_SIZE);

    if (m_http.read(url.c_str(), doc, filter)) {
        JsonArray jPrices = doc["prices"];
        bool first(true);
        for (const auto& p : jPrices) {
            if (!first) { // omit the first (oldest), because there are +1 entries in JSON result
                auto f(p[1].as<gecko_t>());
                targetChart->emplace_back(f);
            }
            first = false;
        }
        return !targetChart->empty();
    }
    return false;
}

bool Gecko::fetchCoinDetails(const char* coin, String& symbolInto, String& nameInto) const
{
    String url("https://api.coingecko.com/api/v3/coins/");
    url += coin;
    url += "?localization=false&tickers=false&market_data=false&community_data=false&developer_data=false&sparkline=false";

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
    String url("https://api.coingecko.com/api/v3/simple/price?ids=");
    url += coin;
    url += "&vs_currencies=usd";

    DynamicJsonDocument doc(DYNAMIC_JSON_VALID_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        auto gecko_says = doc[coin]["usd"] | std::numeric_limits<gecko_t>::max();
        return gecko_says != std::numeric_limits<gecko_t>::max();
    }
    return false;
}

bool Gecko::ping() const
{
    DynamicJsonDocument doc(DYNAMIC_JSON_PING_SIZE);

    if (m_http.read("https://api.coingecko.com/api/v3/ping", doc)) {
        const char* gecko_says = doc["gecko_says"] | ""; // "(V3) To the Moon!"
        return strcmp(gecko_says, "(V3) To the Moon!") == 0;
    }
    return false;
}

bool Gecko::coinDetails(const char* coinOrSymbol, String& coinInto, String& symbolInto, String& nameInto) const
{
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
