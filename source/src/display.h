#pragma once
#include "settings.h"
#include "utils.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

#define TFT_BL PIN_D2 // Backlight PWM

class Gecko;

class Display {
public:
    Display(Gecko& gecko, const Settings& settings);

    void begin();
    void loop();

    void showCoin();
    void showAPQR();
    void showUpdateQR();
    void showSettingsQR();
    void showTest();
    void showAPIOK();
    void showAPIFailed();

    void showUpdated();
    void showNotUpdated();

private:
    void wifiConnect();
    void renderTitle();
    void heartbeat();
    void renderCoin();
    Settings::ChartPeriod nextChartPeriod() const;
    bool renderChart(Settings::ChartPeriod chartPeriod);
    void chartFailed();

    bool drawBmp(const String& filename, int16_t x, int16_t y);

    enum class Screen {
        NONE,
        AP_QR,
        UPDATE_QR,
        API_OK,
        API_FAILED,
        SETTINGS_QR,
        COIN
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
    uint32_t m_display_start { 0 };
    bool m_show_api_ok { true };

    bool m_shows_wifi_not_connected { false };
};
