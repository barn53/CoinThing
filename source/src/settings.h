#pragma once
#include "utils.h"

#define SETTINGS_FILE "/settings.json"
#define BRIGHTNESS_FILE "/brightness.json"

class Settings {
public:
    enum class SecondCurrencyShow : uint8_t {
        VALUE = 0,
        MARKETCAP,
        VOLUME
    };

    enum class ChartStyle : uint8_t {
        SIMPLE = 0,
        HIGH_LOW,
        HIGH_LOW_FIRST_LAST
    };

    enum class Swap : uint8_t {
        INTERVAL_1 = 0, // 5s (chart), 10s (coin)
        INTERVAL_2, // 30s
        INTERVAL_3, // 1min
        INTERVAL_4 // 5min
    };

    enum ChartPeriod : uint8_t {
        PERIOD_NONE = 0,
        PERIOD_24_H = 1,
        PERIOD_48_H = 2,
        PERIOD_30_D = 4,
        PERIOD_60_D = 8,
        ALL_PERIODS = PERIOD_24_H + PERIOD_48_H + PERIOD_30_D + PERIOD_60_D
    };

    enum class Mode : uint8_t {
        ONE_COIN = 1,
        TWO_COINS,
        MULTIPLE_COINS,
        UNDEFINED
    };

    struct Coin {
        String id;
        String symbol;
        String name;
    };

    struct Currency {
        String currency;
        String symbol;
    };

    Settings();

    void begin();
    void set(const char* json);
    void read();
    void deleteFile(bool brightness) const;

    const String& coin(uint32_t index) const;
    const String& name(uint32_t index) const;
    const String& symbol(uint32_t index) const;

    const String& currency() const;
    const String& currencySymbol() const;
    const String& currency2() const;
    const String& currency2Symbol() const;

    CurrencySymbolPosition currencySymbolPosition() const { return m_currency_symbol_position; }

    Mode mode() const { return m_mode; }
    uint32_t numberCoins() const;
    SecondCurrencyShow secondCurrencyShow() const { return m_second_currency_show; }
    NumberFormat numberFormat() const { return m_number_format; }
    SmallDecimalNumberFormat smallDecimalNumberFormat() const { return m_small_decimal_number; }
    uint8_t chartPeriod() const { return m_chart_period; }
    Swap swapInterval() const { return m_swap_interval; }
    ChartStyle chartStyle() const { return m_chart_style; }
    bool heartbeat() const { return m_heartbeat; }
    bool valid() const;

    void readBrightness();
    void setBrightness(uint8_t b);
    uint8_t brightness() const;

    uint32_t lastChange() const { return m_last_change; }

    static void setColorSet(uint8_t colorSet);
    static uint8_t getColorSet();

    static void setFakeGeckoServer(String address);
    static bool isFakeGeckoServer();
    static String getGeckoServer(bool onProAPI);
    static String getSettings();

    static uint8_t handlePowerupSequenceForResetBegin();
    static void handlePowerupSequenceForResetEnd();
    void handlePowerupSequenceForResetEnd(uint8_t powerupSequenceCounter);

private:
    bool set(DynamicJsonDocument& doc);
    void write() const;
    void trace() const;

    uint32_t validCoinIndex(uint32_t index) const;

    Mode m_mode { Mode::UNDEFINED };

    std::vector<Coin> m_coins;
    std::array<Currency, 2> m_currencies;
    CurrencySymbolPosition m_currency_symbol_position { CurrencySymbolPosition::LEADING };

    SecondCurrencyShow m_second_currency_show { SecondCurrencyShow::VALUE };
    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    SmallDecimalNumberFormat m_small_decimal_number { SmallDecimalNumberFormat::NORMAL };
    uint8_t m_chart_period { ChartPeriod::PERIOD_24_H };
    Swap m_swap_interval { Swap::INTERVAL_1 };
    ChartStyle m_chart_style { ChartStyle::SIMPLE };
    bool m_heartbeat { true };

    uint8_t m_brightness { std::numeric_limits<uint8_t>::max() };

    uint32_t m_last_change { 0 };
};
