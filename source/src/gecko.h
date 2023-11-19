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

    const String& getCheckInfo() const { return m_check_info; }
    const String& getCheckError() const { return m_check_error; }

    bool price(uint32_t coinIndex, gecko_t& price, gecko_t& price2, gecko_t& change_pct, gecko_t& market_cap, gecko_t& volume);
    bool twoPrices(gecko_t& price_1, gecko_t& price2_1, gecko_t& change_pct_1, gecko_t& market_cap_1, gecko_t& volume_1,
        gecko_t& price_2, gecko_t& price2_2, gecko_t& change_pct_2, gecko_t& market_cap_2, gecko_t& volume_2);

    const std::vector<gecko_t>& chart_48h(uint32_t coinIndex, bool& refetched);
    const std::vector<gecko_t>& chart_60d(uint32_t coinIndex, bool& refetched);

    uint32_t lastPriceFetch() const { return m_last_price_fetch; }

    bool prefetch(uint32_t coinIndex, Settings::ChartPeriod chartPeriod);

    int getLastHttpCode() const;
    size_t getHttpCount() const;

    bool onProxy() const { return m_on_proxy; }
    bool hadProblemsWithProxy() const { return m_had_problems_with_proxy; }

    bool onProAPI() const { return m_on_pro_api; }
    void enableProAPI(bool enable) { m_pro_api_enabled = enable; };
    bool proAPIEnabled() const { return m_pro_api_enabled; };
    bool hadProblemsWithProApi() const { return m_had_problems_with_pro_api; }

    bool increaseIntervalDueToHTTP429() const { return m_increase_interval_due_to_http_429; }
    uint8_t recoverFromHTTP429() const;

    bool recentlyHTTP429() const { return m_last_http_429 > 0; }
    uint32_t lastHttp429Persist() const { return m_last_http_429_persist; }
    size_t switchToProCount() const { return m_switch_to_pro_count; }
    size_t http429PauseCount() const { return m_http_429_pause_count; }
    size_t http429Count() const { return m_http_429_count; }
    size_t proApiRequestsCount() const { return m_pro_api_requests_count; }
    size_t http403Count() const { return m_http_403_count; }

private:
    String getGeckoServer();

    bool fetchCoinPriceChange(uint32_t coinIndex);
    bool fetchTwoCoinsPriceChange();
    bool fetchCoinChart(uint32_t coinIndex, Settings::ChartPeriod type);
    uint32_t getPriceInterval() const;
    uint32_t getChart48hInterval() const;
    uint32_t getChart60dInterval() const;
    void appendProAPIKey(String& url) const;
    void resetFetchIssue();
    void handleFetchIssue();

    void init();

    bool m_succeeded { false };

    bool m_on_proxy { false };
    bool m_had_problems_with_proxy { false };

    bool m_on_pro_api { false };
    bool m_pro_api_enabled { false };
    bool m_had_problems_with_pro_api { false };
    String m_pro_api_key;
    bool m_increase_interval_due_to_http_429 { false };
    uint32_t m_last_http_429 { 0 };
    uint32_t m_last_http_429_persist { 0 };
    String m_gecko_server;

    size_t m_switch_to_pro_count { 0 };
    size_t m_http_429_pause_count { 0 };
    size_t m_http_429_count { 0 };
    size_t m_pro_api_requests_count { 0 };

    size_t m_http_403_count { 0 };

    gecko_t m_price { 0. };
    gecko_t m_price2 { 0. };
    gecko_t m_change_pct { 0. };
    gecko_t m_market_cap { 0. };
    gecko_t m_volume { 0. };
    // for two coins:
    gecko_t m_price_2 { 0. };
    gecko_t m_price2_2 { 0. };
    gecko_t m_change_pct_2 { 0. };
    gecko_t m_market_cap_2 { 0. };
    gecko_t m_volume_2 { 0. };

    std::vector<gecko_t> m_chart_48h;
    std::vector<gecko_t> m_chart_60d;

    uint32_t m_last_price_fetch { 0 };
    uint32_t m_last_chart_48h_fetch { 0 };
    uint32_t m_last_chart_60d_fetch { 0 };
    uint32_t m_last_seen_settings { 0 };
    uint32_t m_last_ping { 0 };

    String m_check_info;
    String m_check_error;
};
