#pragma once
#include "settings.h"
#include "utils.h"

class HttpJson;

bool isCoin(const char* coin);

class Gecko {

public:
    Gecko(HttpJson& http, Settings& settings);

    void begin();
    void loop();

    bool ping();
    bool succeeded() const { return m_succeeded; }

    void price(uint32_t coinIndex, gecko_t& price, gecko_t& price2, gecko_t& change_pct);
    void twoPrices(gecko_t& price_1, gecko_t& price2_1, gecko_t& change_pct_1,
        gecko_t& price_2, gecko_t& price2_2, gecko_t& change_pct_2);
    const std::vector<gecko_t>& chart_48h(uint32_t coinIndex, bool& refetched);
    const std::vector<gecko_t>& chart_60d(uint32_t coinIndex, bool& refetched);

    uint32_t lastPriceFetch() const { return m_last_price_fetch; }

    void prefetch(uint32_t coinIndex, Settings::ChartPeriod chartPeriod);

    HttpJson& getHttpJson() { return m_http; }
    int getLastHttpCode() const;
    size_t getHttpCount() const;

private:
    bool fetchCoinPriceChange(uint32_t coinIndex);
    bool fetchTwoCoinsPriceChange();
    bool fetchCoinChart(uint32_t coinIndex, Settings::ChartPeriod type);

    bool m_succeeded { false };

    String m_gecko_server;

    gecko_t m_price { 0. };
    gecko_t m_price2 { 0. };
    gecko_t m_change_pct { 0. };
    // for two coins:
    gecko_t m_price_2 { 0. };
    gecko_t m_price2_2 { 0. };
    gecko_t m_change_pct_2 { 0. };
    std::vector<gecko_t> m_chart_48h;
    std::vector<gecko_t> m_chart_60d;

    uint32_t m_last_price_fetch { 0 };
    uint32_t m_last_chart_48h_fetch { 0 };
    uint32_t m_last_chart_60d_fetch { 0 };
    uint32_t m_last_seen_settings { 0 };
    uint32_t m_last_ping { 0 };

    HttpJson& m_http;
    Settings& m_settings;
};
