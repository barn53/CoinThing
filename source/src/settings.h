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
    enum Chart : uint8_t {
        CHART_24_H = 1,
        CHART_48_H = 2,
        CHART_30_D = 4,
        CHART_60_D = 8,
        CHART_ALL = CHART_24_H + CHART_48_H + CHART_30_D + CHART_60_D
    };

    Settings();

    bool begin(const Gecko& gecko);
    bool read(const Gecko& gecko);
    void write();
    const char* coin() const { return m_coin.c_str(); }
    const char* currency() const { return m_currency.c_str(); }
    NumberFormat numberFormat() const { return m_number_format; }
    uint8_t chart() const { return m_chart; }
    bool heartbeat() const { return m_heartbeat; }
    const char* name() const { return m_name.c_str(); }
    const char* symbol() const { return m_symbol.c_str(); }
    uint8_t brightness() const;
    bool valid() const { return m_valid; }

    Status set(const Gecko& gecko, const char* coin, const char* currency, uint8_t number_format, uint8_t chart, bool heartbeat);
    bool setBrightness(uint8_t b);

    uint32_t lastChange() const { return m_lastChange; }

private:
    String m_coin;
    String m_currency;
    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    uint8_t m_chart { Chart::CHART_24_H };
    bool m_heartbeat { true };
    String m_name;
    String m_symbol;
    uint8_t m_brightness { std::numeric_limits<uint8_t>::max() };
    bool m_valid { false };

    uint32_t m_lastChange { 0 };
};
