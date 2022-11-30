#pragma once
#include "settings.h"
#include "utils.h"
#include <TFT_eSPI.h>

#define TFT_BL PIN_D2 // Backlight PWM

class Display {
public:
    Display();

    void begin(uint8_t powerupSequenceCounter);
    void loop();

    void showAPQR();
    void showUpdateQR();

    void showPrepareUpdate(bool failed);
    void showUpdated();
    void showNotUpdated();

    void enableOnScreenDebug(bool enable) { m_on_screen_debug_enabled = enable; };

    static bool drawBmp(const String& filename, TFT_eSPI& tft, int16_t x, int16_t y);

private:
    void wifiConnect();
    void renderTitle();
    void showRecover(uint8_t pct);
    void heartbeat();
    void showCoin();
    void showTwoCoins();
    void showMultipleCoins();
    void showCoinThing(String& msg, uint16_t y = 30, bool unloadFont = true);
    void showAPIInfo(String& msg);
    void renderCoin();
    void renderTwoCoins();
    bool renderChart(Settings::ChartPeriod chartPeriod);
    void chartFailed();

    void showOnScreenDebug();

    void showSettingsQR();
    void showAPIOK();
    void showAPIFailed();

    Settings::ChartPeriod nextChartPeriod() const;
    void nextCoinID();

    enum class Screen {
        NONE,
        AP_QR,
        UPDATE_QR,
        API_OK,
        API_FAILED,
        SETTINGS_QR,
        COIN
    };

    enum class CoinDownUpState {
        CHANGE_UNDER_1_PCT = 0,
        CHANGE_PLUS_1_TO_5_PCT = 1,
        CHANGE_PLUS_MORE_THAN_5_PCT = 2,
        CHANGE_MINUS_1_TO_5_PCT = 3,
        CHANGE_MINUS_MORE_THAN_5_PCT = 4,
    };

    Screen m_last_screen { Screen::NONE };
    Settings::ChartPeriod m_last_chart_period { Settings::ChartPeriod::PERIOD_NONE };

    uint32_t m_last_seen_settings { 0 };
    uint32_t m_last_heartbeat { 0 };
    uint8_t m_heart_beat_count { 0 };
    uint32_t m_last_price_update { 0 };
    uint32_t m_last_chart_update { 0 };
    uint32_t m_display_start { 0 };
    bool m_show_api_ok { true };
    bool m_previous_prefetch_failed { false };

    uint32_t m_current_coin_index { std::numeric_limits<uint32_t>::max() };
    CoinDownUpState m_coin_down_up[10] { CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT,
        CoinDownUpState::CHANGE_UNDER_1_PCT };

    uint32_t m_last_coin_swap { 0 };

    uint32_t m_last_on_screen_debug_update { 0 };
    bool m_on_screen_debug_enabled { false };

    bool m_shows_wifi_not_connected { false };
    bool m_shows_recover { false };

    static uint16_t RED565;
    static uint16_t GREEN565;
    static uint16_t GREY_LEVEL2;
    static uint16_t PERIOD_COLOR;
    static uint16_t CURRENT_COIN_DOT_COLOR;
    static uint16_t COIN_DOWN_COLOR;
    static uint16_t COIN_UP_COLOR;
    static uint16_t CHART_VERTICAL_LINE_COLOR;
    static uint16_t CHART_FIRST_COLOR;
    static uint16_t CHART_LAST_COLOR;
    static uint16_t CHART_HIGH_COLOR;
    static uint16_t CHART_LOW_COLOR;
    static uint16_t CHART_BOX_BG;
    static uint16_t CHART_BOX_MARGIN_COLOR;
};
