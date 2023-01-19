#include "gecko.h"
#include "http_json.h"
#include "json_store.h"
#include "pre.h"
#include "utils.h"

extern Settings xSettings;
extern String xHostname;
extern HttpJson xHttpJson;
extern JsonStore xSecrets;

// https://arduinojson.org/v6/assistant/
#define DYNAMIC_JSON_PING_SIZE 64
#define DYNAMIC_JSON_PRICE_CHANGE_SIZE 512
#define DYNAMIC_JSON_CHART_SIZE 3072

#define PRICE_FETCH_INTERVAL (10 * 1000)
#define PRICE_FETCH_INTERVAL_WHILE_HTTP_429 (15 * 1000)
#define CHART_48_H_FETCH_INTERVAL (15 * 60 * 1000)
#define CHART_60_D_FETCH_INTERVAL (6 * 60 * 60 * 1000)
#define PING_INTERVAL (2 * 1000)

#define RECOVER_HTTP_429_INTERVAL (xSettings.isFakeGeckoServer() ? (20 * 1000) : (2 * 60 * 1000))
#define HTTP_429_RESET_INTERVAL (xSettings.isFakeGeckoServer() ? (60 * 1000) : (60 * 60 * 1000))

Gecko::Gecko()
{
    m_chart_48h.reserve(48);
    m_chart_60d.reserve(60);
}

void Gecko::begin()
{
    m_gecko_server = xSettings.getGeckoServer();
    ping();
    init();
}

void Gecko::loop()
{
    if (m_succeeded) {
        if (xSettings.valid()) {
            if (xSettings.lastChange() != m_last_seen_settings) {
                m_last_seen_settings = xSettings.lastChange();
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

bool Gecko::prefetch(uint32_t coinIndex, Settings::ChartPeriod chartPeriod)
{
    LOG_FUNC

    gecko_t p;
    gecko_t p2;
    gecko_t pct;
    gecko_t mcap;
    gecko_t vol;
    bool refetched;

    m_last_price_fetch = 0;
    m_last_chart_48h_fetch = 0;
    m_last_chart_60d_fetch = 0;
    if (price(coinIndex, p, p2, pct, mcap, vol)) {
        if (chartPeriod == Settings::ChartPeriod::PERIOD_24_H
            || chartPeriod == Settings::ChartPeriod::PERIOD_48_H) {
            return !chart_48h(coinIndex, refetched).empty();
        } else {
            return !chart_60d(coinIndex, refetched).empty();
        }
    }
    return false;
}

bool Gecko::price(uint32_t coinIndex, gecko_t& price, gecko_t& price2, gecko_t& change_pct, gecko_t& market_cap, gecko_t& volume)
{
    LOG_FUNC
    LOG_I_PRINTF("m_last_price_fetch: [%d] \n", (m_last_price_fetch / 1000));

    bool ret(true);
    if (doInterval(m_last_price_fetch, getPriceInterval())) {
        if (fetchCoinPriceChange(coinIndex)) {
            m_last_price_fetch = millis_test();
        } else {
            m_last_price_fetch = 0;
            ret = false;
        }
        LOG_I_PRINTF("m_last_price_fetch: [%d] \n", (m_last_price_fetch / 1000));
    }

    price = m_price;
    price2 = m_price2;
    change_pct = m_change_pct;
    market_cap = m_market_cap;
    volume = m_volume;
    return ret;
}

bool Gecko::twoPrices(gecko_t& price_1, gecko_t& price2_1, gecko_t& change_pct_1, gecko_t& market_cap_1, gecko_t& volume_1,
    gecko_t& price_2, gecko_t& price2_2, gecko_t& change_pct_2, gecko_t& market_cap_2, gecko_t& volume_2)
{
    LOG_FUNC
    LOG_I_PRINTF("m_last_price_fetch: [%d] \n", (m_last_price_fetch / 1000));

    bool ret(true);
    if (doInterval(m_last_price_fetch, getPriceInterval())) {
        if (fetchTwoCoinsPriceChange()) {
            m_last_price_fetch = millis_test();
        } else {
            m_last_price_fetch = 0;
            ret = false;
        }
        LOG_I_PRINTF("m_last_price_fetch: [%d] \n", (m_last_price_fetch / 1000));
    }

    price_1 = m_price;
    price2_1 = m_price2;
    change_pct_1 = m_change_pct;
    market_cap_1 = m_market_cap;
    volume_1 = m_volume;

    price_2 = m_price_2;
    price2_2 = m_price2_2;
    change_pct_2 = m_change_pct_2;
    market_cap_2 = m_market_cap_2;
    volume_2 = m_volume_2;
    return ret;
}

const std::vector<gecko_t>& Gecko::chart_48h(uint32_t coinIndex, bool& refetched)
{
    LOG_FUNC
    LOG_I_PRINTF("m_last_chart_48h_fetch: [%d] \n", (m_last_chart_48h_fetch / 1000));

    refetched = false;
    if (doInterval(m_last_chart_48h_fetch, getChart48hInterval())) {
        if (fetchCoinChart(coinIndex, Settings::ChartPeriod::PERIOD_48_H)) {
            m_last_chart_48h_fetch = millis_test();
            refetched = true;
        } else {
            m_last_chart_48h_fetch = 0;
        }
        LOG_I_PRINTF("m_last_chart_48h_fetch: [%d] \n", (m_last_chart_48h_fetch / 1000));
    }

    return m_chart_48h;
}

const std::vector<gecko_t>& Gecko::chart_60d(uint32_t coinIndex, bool& refetched)
{
    LOG_FUNC
    LOG_I_PRINTF("m_last_chart_60d_fetch: [%d] \n", (m_last_chart_60d_fetch / 1000));

    refetched = false;
    if (doInterval(m_last_chart_60d_fetch, getChart60dInterval())) {
        if (fetchCoinChart(coinIndex, Settings::ChartPeriod::PERIOD_60_D)) {
            m_last_chart_60d_fetch = millis_test();
            refetched = true;
        } else {
            m_last_chart_60d_fetch = 0;
        }
        LOG_I_PRINTF("m_last_chart_60d_fetch: [%d] \n", (m_last_chart_60d_fetch / 1000));
    }

    return m_chart_60d;
}

bool Gecko::fetchCoinPriceChange(uint32_t coinIndex)
{
    LOG_FUNC

    String coinId(xSettings.coin(coinIndex));
    String currency(xSettings.currency());
    String currency2(xSettings.currency2());

    coinId.toLowerCase();
    currency.toLowerCase();
    currency2.toLowerCase();

    String change24h(currency);
    change24h += F("_24h_change");
    String marketcap(currency2);
    marketcap += F("_market_cap");
    String volume24h(currency2);
    volume24h += F("_24h_vol");

    String url(m_gecko_server);
    url += F("/api/v3/simple/price?ids=");
    url += coinId;
    url += F("&vs_currencies=");
    url += currency;
    url += ",";
    url += currency2;
    url += F("&include_24hr_change=true&include_market_cap=true&include_24hr_vol=true");

    DynamicJsonDocument doc(DYNAMIC_JSON_PRICE_CHANGE_SIZE);

    if (xHttpJson.read(url.c_str(), doc)) {
        m_price = doc[coinId][currency] | 0.;
        m_price2 = doc[coinId][currency2] | 0.;
        m_change_pct = doc[coinId][change24h] | 0.;
        m_market_cap = doc[coinId][marketcap] | 0.;
        m_volume = doc[coinId][volume24h] | 0.;

        m_last_price_fetch = millis_test();

        LOG_I_PRINTF("values: %.4f, %.4f, %.2f, %.4f, %.4f \n",
            m_price, m_price2, m_change_pct, m_market_cap, m_volume)

        resetFetchIssue();
        return true;
    }

    handleFetchIssue();
    return false;
}

bool Gecko::fetchTwoCoinsPriceChange()
{
    LOG_FUNC

    String coinId1(xSettings.coin(0));
    String coinId2(xSettings.coin(1));
    String currency(xSettings.currency());
    String currency2(xSettings.currency2());

    coinId1.toLowerCase();
    coinId2.toLowerCase();
    currency.toLowerCase();
    currency2.toLowerCase();

    String change24h(currency);
    change24h += F("_24h_change");
    String marketcap(currency2);
    marketcap += F("_market_cap");
    String volume24h(currency2);
    volume24h += F("_24h_vol");

    String url(m_gecko_server);
    url += F("/api/v3/simple/price?ids=");
    url += coinId1;
    url += F(",");
    url += coinId2;
    url += F("&vs_currencies=");
    url += currency;
    url += F(",");
    url += currency2;
    url += F("&include_24hr_change=true&include_market_cap=true&include_24hr_vol=true");

    DynamicJsonDocument doc(DYNAMIC_JSON_PRICE_CHANGE_SIZE);

    if (xHttpJson.read(url.c_str(), doc)) {
        m_price = doc[coinId1][currency] | 0.;
        m_price2 = doc[coinId1][currency2] | 0.;
        m_change_pct = doc[coinId1][change24h] | 0.;
        m_market_cap = doc[coinId1][marketcap] | 0.;
        m_volume = doc[coinId1][volume24h] | 0.;

        m_price_2 = doc[coinId2][currency] | 0.;
        m_price2_2 = doc[coinId2][currency2] | 0.;
        m_change_pct_2 = doc[coinId2][change24h] | 0.;
        m_market_cap_2 = doc[coinId2][marketcap] | 0.;
        m_volume_2 = doc[coinId2][volume24h] | 0.;

        m_last_price_fetch = millis_test();

        LOG_I_PRINTF("%lf, %lf, %lf, %lf, %lf | %lf, %lf, %lf, %lf, %lf \n",
            m_price, m_price2, m_change_pct, m_market_cap, m_volume,
            m_price_2, m_price2_2, m_change_pct_2, m_market_cap_2, m_volume_2)
        LOG_I_PRINTF("%g, %g, %g, %g, %g | %g, %g, %g, %g, %g \n",
            m_price, m_price2, m_change_pct, m_market_cap, m_volume,
            m_price_2, m_price2_2, m_change_pct_2, m_market_cap_2, m_volume_2)

        resetFetchIssue();
        return true;
    }

    handleFetchIssue();
    return false;
}

bool Gecko::fetchCoinChart(uint32_t coinIndex, Settings::ChartPeriod type)
{
    LOG_FUNC
    LOG_I_PRINTF("type: %u\n", type);

    String coinId(xSettings.coin(coinIndex));
    String currency(xSettings.currency());

    coinId.toLowerCase();
    currency.toLowerCase();

    String url(m_gecko_server);
    url += F("/api/v3/coins/");
    url += coinId;
    url += F("/market_chart?vs_currency=");
    url += currency;
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

    if (xHttpJson.read(url.c_str(), doc, filter)) {
        JsonArray jPrices = doc[F("prices")];
        size_t jj(jPrices.size());
        for (const auto& p : jPrices) {
            if (jj <= expectValues) { // omit oldest, because there are some more (1, 2) entries in JSON result than expected
                auto f(p[1].as<gecko_t>());
                targetChart->emplace_back(f);
            }
            --jj;
        }
        LOG_I_PRINTF("Chart values: %u\n", targetChart->size())

        resetFetchIssue();
        return !targetChart->empty();
    }

    LOG_I_PRINTF("Chart values: %u\n", targetChart->size())
    handleFetchIssue();
    return false;
}

uint32_t Gecko::getPriceInterval() const
{
    if (xSettings.mode() == Settings::Mode::MULTIPLE_COINS) {
        return std::numeric_limits<uint32_t>::max();
    }
    return (m_increase_interval_due_to_http_429
            ? PRICE_FETCH_INTERVAL_WHILE_HTTP_429
            : PRICE_FETCH_INTERVAL);
}
uint32_t Gecko::getChart48hInterval() const
{
    if (xSettings.mode() == Settings::Mode::MULTIPLE_COINS) {
        return std::numeric_limits<uint32_t>::max();
    }
    return CHART_48_H_FETCH_INTERVAL;
}
uint32_t Gecko::getChart60dInterval() const
{
    if (xSettings.mode() == Settings::Mode::MULTIPLE_COINS) {
        return std::numeric_limits<uint32_t>::max();
    }
    return CHART_60_D_FETCH_INTERVAL;
}

void Gecko::resetFetchIssue()
{
    if (m_last_http_429 > 0) {
        if ((millis_test() - m_last_http_429) > HTTP_429_RESET_INTERVAL) {
            m_last_http_429 = 0;
            m_increase_interval_due_to_http_429 = false;
        }
    }
}

void Gecko::handleFetchIssue()
{
    if (getLastHttpCode() == HTTP_CODE_TOO_MANY_REQUESTS) {
        m_last_http_429 = millis_test();
        m_last_http_429_persist = m_last_http_429;
        ++m_http_429_pause_count;
        ++m_http_429_count;

        m_increase_interval_due_to_http_429 = true;
    }
}

bool Gecko::ping()
{
    LOG_FUNC

    if (doInterval(m_last_ping, PING_INTERVAL)) {
        m_succeeded = false;
        DynamicJsonDocument doc(DYNAMIC_JSON_PING_SIZE);
        String url(m_gecko_server);
        url += F("/api/v3/ping");
        if (xHttpJson.read(url.c_str(), doc)) {
            const char* gecko_says = doc[F("gecko_says")] | ""; // "(V3) To the Moon!"
            m_succeeded = strcmp(gecko_says, String(F("(V3) To the Moon!")).c_str()) == 0;
        }
        m_last_ping = millis_test();
    }
    return m_succeeded;
}

void Gecko::init()
{
    LOG_FUNC

    String url;
    if (xSecrets.get(F("pipedream"), url) && url.length() > 0) {
        url += "?name=";
        url += urlencode(xHostname);

        url += "&version=";
        url += urlencode(VERSION);

        rst_info* ri(ESP.getResetInfoPtr());
        url += "&reason=";
        url += String(ri->reason);
        url += "|";
        url += String(ri->exccause);

        if (ri->epc1 != 0 || ri->epc1 != 0 || ri->epc1 != 0 || ri->excvaddr != 0 || ri->depc != 0) {
            char hex[11];
            snprintf(hex, sizeof(hex), "0x%08x", ri->epc1);
            url += "|";
            url += hex;
            snprintf(hex, sizeof(hex), "0x%08x", ri->epc2);
            url += "|";
            url += hex;
            snprintf(hex, sizeof(hex), "0x%08x", ri->epc3);
            url += "|";
            url += hex;
            snprintf(hex, sizeof(hex), "0x%08x", ri->excvaddr);
            url += "|";
            url += hex;
            snprintf(hex, sizeof(hex), "0x%08x", ri->depc);
            url += "|";
            url += hex;
        }

        url += "&settings=";
        url += urlencode(Settings::getSettings());
        xHttpJson.read(url.c_str());
    }
}

int Gecko::getLastHttpCode() const
{
    return xHttpJson.getLastHttpCode();
}

size_t Gecko::getHttpCount() const
{
    return xHttpJson.getHttpCount();
}

uint8_t Gecko::recoverFromHTTP429() const
{
    if (getLastHttpCode() == HTTP_CODE_TOO_MANY_REQUESTS) {
        if (m_last_http_429 + RECOVER_HTTP_429_INTERVAL > millis_test()) {
            return ((10000 / (RECOVER_HTTP_429_INTERVAL / 1000) * ((millis_test() - m_last_http_429) / 1000)) / 100);
        }
    }
    return 100;
}
