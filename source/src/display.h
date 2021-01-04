#pragma once
#include <Arduino.h>
#include <TFT_eFEX.h>
#include <TFT_eSPI.h>

class Settings;

class Display {
public:
    Display(Settings& setttings);

    void begin();
    void loop();

    void showCoin();
    void showAPQR();
    void showSettingsQR();
    void showAPIOK();
    void showAPIFailed();

private:
    void heartbeat();
    void coin(double price, double price_usd, double change, uint16_t color);
    void chart(const std::vector<double>& prices, double max, double min, uint16_t color);
    void rewrite();
    void chartFailed();

    enum class ShowedScreen {
        NONE,
        AP_QR,
        API_OK,
        API_FAILED,
        SETTINGS_QR,
        COIN
    };

    Settings& m_settings;
    TFT_eSPI m_tft;
    TFT_eFEX m_fex;
    TFT_eSprite m_spr;

    ShowedScreen m_lastScreen { ShowedScreen::NONE };
    unsigned long m_lastHeartbeat { 0 };
    uint8_t m_heartbeat { 0 };
    unsigned long m_lastUpdate { 0 };
    unsigned long m_lastChartUpdate { 0 };
};
