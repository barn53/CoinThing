#pragma once
#include "settings.h"
#include "utils.h"

class Gecko {

public:
    Gecko();

    void begin();
    void loop();

    bool ping();
    bool succeeded() const { return m_succeeded; }

    bool price(uint32_t coinIndex, gecko_t& price, gecko_t& price2, gecko_t& change_pct);
    bool twoPrices(gecko_t& price_1, gecko_t& price2_1, gecko_t& change_pct_1,
        gecko_t& price_2, gecko_t& price2_2, gecko_t& change_pct_2);
    const std::vector<gecko_t>& chart_48h(uint32_t coinIndex, bool& refetched);
    const std::vector<gecko_t>& chart_60d(uint32_t coinIndex, bool& refetched);

    uint32_t lastPriceFetch() const { return m_last_price_fetch; }

    bool prefetch(uint32_t coinIndex, Settings::ChartPeriod chartPeriod);

    int getLastHttpCode() const;
    size_t getHttpCount() const;

    bool increaseIntervalDueToHTTP429() const { return m_increase_interval_due_to_http_429; }
    uint8_t recoverFromHTTP429() const;

    //   remove
    bool recentlyHTTP429() const { return m_last_http_429 > 0; }
    size_t http429PauseCount() const { return m_http_429_pause_count; }
    //   /remove

private:
    bool fetchCoinPriceChange(uint32_t coinIndex);
    bool fetchTwoCoinsPriceChange();
    bool fetchCoinChart(uint32_t coinIndex, Settings::ChartPeriod type);

    uint32_t getPriceInterval() const;
    uint32_t getChart48hInterval() const;
    uint32_t getChart60dInterval() const;

    void resetFetchIssue();
    void handleFetchIssue();

    void init();

    bool m_succeeded { false };

    bool m_increase_interval_due_to_http_429 { false };
    uint32_t m_last_http_429 { 0 };
    String m_gecko_server;

    size_t m_http_429_pause_count { 0 };

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
};
