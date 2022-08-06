#pragma once
#include "settings.h"
#include "utils.h"
#include <TFT_eSPI.h>

#define TFT_BL PIN_D2 // Backlight PWM

class Gecko;

class Display {
public:
    Display(Gecko& gecko, const Settings& settings);

    void begin(uint8_t powerupSequenceCounter);
    void loop();

    void showAPQR();
    void showUpdateQR();

    void showPrepareUpdate(bool failed);
    void showUpdated();
    void showNotUpdated();

    static bool drawBmp(const String& filename, TFT_eSPI& tft, int16_t x, int16_t y);

    Gecko& getGecko() { return m_gecko; }
    TFT_eSPI& getTFT() { return m_tft; }

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

    void drawWhale();
    void showWhaleTicker(String& msg);
    void showAPIOK();
    void showAPIFailed();
    void showSettingsQR();
    void showCheckInfo();
    void showCheckError();

    Settings::ChartPeriod nextChartPeriod() const;
    void nextCoinID();

    enum class Screen {
        NONE,
        AP_QR,
        UPDATE_QR,
        API_OK,
        API_FAILED,
        SETTINGS_QR,
        COIN,
        CHECK_INFO,
        CHECK_ERROR
    };

    Gecko& m_gecko;
    const Settings& m_settings;
    TFT_eSPI m_tft;

    Screen m_last_screen { Screen::NONE };
    Settings::ChartPeriod m_last_chart_period { Settings::ChartPeriod::PERIOD_NONE };

    uint32_t m_last_seen_settings { 0 };
    uint32_t m_last_heartbeat { 0 };
    uint8_t m_heart_beat_count { 0 };
    uint32_t m_last_price_update { 0 };
    uint32_t m_last_chart_update { 0 };

    uint32_t m_show_api_ok_start { 0 };
    uint32_t m_show_check_info_start { 0 };
    bool m_show_api_ok { true };
    bool m_show_check_info { true };

    uint32_t m_current_coin_index { std::numeric_limits<uint32_t>::max() };
    uint32_t m_last_coin_swap { 0 };

    bool m_shows_wifi_not_connected { false };
    bool m_shows_recover { false };

    static uint16_t RED565;
    static uint16_t GREEN565;
    static uint16_t GREY_LEVEL2;
    static uint16_t PERIOD_COLOR;
    static uint16_t CURRENT_COIN_DOT_COLOR;
    static uint16_t CHART_VERTICAL_LINE_COLOR;
    static uint16_t CHART_FIRST_COLOR;
    static uint16_t CHART_LAST_COLOR;
    static uint16_t CHART_HIGH_COLOR;
    static uint16_t CHART_LOW_COLOR;
    static uint16_t CHART_BOX_BG;
    static uint16_t CHART_BOX_MARGIN_COLOR;
};
