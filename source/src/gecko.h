#pragma once
#include "settings.h"
#include "utils.h"
#include <Arduino.h>

class HttpJson;

bool isCoin(const char* coin);

class Gecko {

public:
    Gecko(HttpJson& http, Settings& settings);

    void begin();
    void loop();

    bool ping();
    bool succeeded() const { return m_succeeded; }


    void price(gecko_t& price, gecko_t& price2, gecko_t& change_pct);
    const std::vector<gecko_t>& chart_48h(bool& refetched);
    const std::vector<gecko_t>& chart_60d(bool& refetched);

    uint32_t lastPriceFetch() const { return m_last_price_fetch; }

private:
    bool fetchCoinPriceChange();
    bool fetchCoinChart(Settings::ChartPeriod type);

    bool m_succeeded { false };

    gecko_t m_price;
    gecko_t m_price2;
    gecko_t m_change_pct;
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
