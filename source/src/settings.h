#pragma once
#include "utils.h"
#include <Arduino.h>

#define USER_CONFIG "/settings.json"
#define DYNAMIC_JSON_CONFIG_SIZE 256

class Gecko;

class Settings {
public:
    enum class Status : uint8_t {
        OK = 0,
        COIN_INVALID,
        CURRENCY_INVALID,
    };
    enum class Chart : uint8_t {
        CHART_24_H = 0,
        CHART_30_D,
        CHART_BOTH
    };

    Settings(Gecko& http);

    bool begin();
    bool read();
    void write() const;
    const char* coin() const { return m_coin.c_str(); }
    const char* currency() const { return m_currency.c_str(); }
    NumberFormat numberFormat() const { return m_number_format; }
    Chart chart() const { return m_chart; }
    bool heartbeat() const { return m_heartbeat; }
    const char* name() const { return m_name.c_str(); }
    const char* symbol() const { return m_symbol.c_str(); }
    bool valid() const { return m_valid; }

    void displayed(bool d) { m_displayed = d; }
    bool displayed() const { return m_displayed; }

    Status set(const char* coin, const char* currency, uint8_t number_format, uint8_t chart, bool heartbeat);

    Gecko& getGecko() { return m_gecko; }

private:
    String m_coin;
    String m_currency;
    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    Chart m_chart { Chart::CHART_24_H };
    bool m_heartbeat { true };
    String m_name;
    String m_symbol;
    bool m_valid { false };
    bool m_displayed { false };

    Gecko& m_gecko;
};
