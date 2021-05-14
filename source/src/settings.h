#pragma once
#include "utils.h"
#include <Arduino.h>

#define SETTINGS_FILE "/settings.json"
#define BRIGHTNESS_FILE "/brightness.json"

class Gecko;

class Settings {
public:
    class Coin {
    public:
        String id;
        String symbol;
        String name;
    };

    enum class ChartStyle : uint8_t {
        SIMPLE = 0,
        HIGH_LOW,
        HIGH_LOW_FIRST_LAST
    };
    enum class SwapInterval : uint8_t {
        SEC_5 = 0,
        SEC_30,
        MIN_1,
        MIN_5
    };

    enum ChartPeriod : uint8_t {
        PERIOD_NONE = 0,
        PERIOD_24_H = 1,
        PERIOD_48_H = 2,
        PERIOD_30_D = 4,
        PERIOD_60_D = 8,
        ALL_PERIODS = PERIOD_24_H + PERIOD_48_H + PERIOD_30_D + PERIOD_60_D
    };

    enum Mode : uint8_t {
        ONE_COIN = 0,
        TWO_COINS,
        MULTIPLE_COINS
    };

    Settings();

    bool begin(const Gecko& gecko);
    bool read(const Gecko& gecko);
    void write();
    void deleteFile();

    const char* coin() const { return m_coin.c_str(); }
    const char* currency() const { return m_currency.c_str(); }
    const char* currency2() const { return m_currency2.c_str(); }
    NumberFormat numberFormat() const { return m_number_format; }
    uint8_t chartPeriod() const { return m_chart_period; }
    SwapInterval swapInterval() const { return m_swap_interval; }
    ChartStyle chartStyle() const { return m_chart_style; }
    bool heartbeat() const { return m_heartbeat; }
    const char* name() const { return m_name.c_str(); }
    const char* symbol() const { return m_symbol.c_str(); }
    uint8_t brightness() const;
    bool valid() const { return m_valid; }

    void set(const Gecko& gecko, const char* json);
    bool setBrightness(uint8_t b);

    uint32_t lastChange() const { return m_last_change; }

private:
    String m_coin;
    String m_currency;
    String m_currency2;
    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    uint8_t m_chart_period { ChartPeriod::PERIOD_24_H };
    SwapInterval m_swap_interval { SwapInterval::SEC_5 };
    ChartStyle m_chart_style { ChartStyle::SIMPLE };
    bool m_heartbeat { true };
    String m_name;
    String m_symbol;
    uint8_t m_brightness { std::numeric_limits<uint8_t>::max() };
    bool m_valid { false };

    uint32_t m_last_change { 0 };
};

class SettingsV12 {
public:
    enum class ChartStyle : uint8_t {
        SIMPLE = 0,
        HIGH_LOW,
        HIGH_LOW_FIRST_LAST
    };
    enum class SwapInterval : uint8_t {
        SEC_5 = 0,
        SEC_30,
        MIN_1,
        MIN_5
    };

    enum ChartPeriod : uint8_t {
        PERIOD_NONE = 0,
        PERIOD_24_H = 1,
        PERIOD_48_H = 2,
        PERIOD_30_D = 4,
        PERIOD_60_D = 8,
        ALL_PERIODS = PERIOD_24_H + PERIOD_48_H + PERIOD_30_D + PERIOD_60_D
    };

    enum Mode : uint8_t {
        ONE_COIN = 0,
        TWO_COINS,
        MULTIPLE_COINS
    };

    struct Coin {
        String id;
        String symbol;
        String name;
    };

    SettingsV12();

    void begin();
    void set(const char* json);
    void read();
    void write() const;
    void deleteFile() const;

    /*
    const char* coin() const { return m_coin.c_str(); }
    const char* name() const { return m_name.c_str(); }
    const char* symbol() const { return m_symbol.c_str(); }
    */

    const char* currency() const { return m_currency.c_str(); }
    const char* currency2() const { return m_currency2.c_str(); }

    NumberFormat numberFormat() const { return m_number_format; }

    uint8_t chartPeriod() const { return m_chart_period; }

    SwapInterval swapInterval() const { return m_swap_interval; }

    ChartStyle chartStyle() const { return m_chart_style; }

    bool heartbeat() const { return m_heartbeat; }

    bool valid() const;

    uint32_t lastChange() const { return m_last_change; }

private:
    void trace() const;

    Mode m_mode { Mode::ONE_COIN };

    std::vector<Coin> m_coins;

    String m_currency;
    String m_currency2;

    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };

    uint8_t m_chart_period { ChartPeriod::PERIOD_24_H };

    SwapInterval m_swap_interval { SwapInterval::SEC_5 };

    ChartStyle m_chart_style { ChartStyle::SIMPLE };

    bool m_heartbeat { true };

    uint32_t m_last_change { 0 };
};
