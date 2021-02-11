#pragma once
#include "settings.h"
#include "utils.h"
#include <Arduino.h>
#include <NeoPixelBus.h>
#include <TFT_eFEX.h>
#include <TFT_eSPI.h>

#define NEO_AND PIN_D1 // enable NeoPixel in
#define TFT_BL PIN_D2 // Backlight PWM

class Gecko;

class Display {
public:
    Display(Gecko& gecko, const Settings& settings);

    void begin();
    void loop();

    void showCoin();
    void showAPQR();
    void showSettingsQR();
    void showAPIOK();
    void showAPIFailed();

private:
    void renderTitle();
    void heartbeat();
    void renderCoin();
    Settings::ChartPeriod nextChartType();
    bool renderChart(Settings::ChartPeriod type);
    void chartFailed();

    enum class Screen {
        NONE,
        AP_QR,
        API_OK,
        API_FAILED,
        SETTINGS_QR,
        COIN
    };

    Gecko& m_gecko;
    const Settings& m_settings;
    TFT_eSPI m_tft;
    TFT_eFEX m_fex;

    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> m_neo { 1 };

    Screen m_last_screen { Screen::NONE };

    uint32_t m_last_seen_settings { 0 };
    uint32_t m_last_heartbeat { 0 };
    uint8_t m_heart_beat_count { 0 };
    uint32_t m_last_price_update { 0 };
    uint32_t m_last_chart_update { 0 };
    Settings::ChartPeriod m_last_chart { Settings::ChartPeriod::PERIOD_NONE };
};
