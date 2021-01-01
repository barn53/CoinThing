#pragma once
#include <Arduino.h>
#include <TFT_eFEX.h>
#include <TFT_eSPI.h>

class Config;

class Display {
public:
    Display(Config& config);

    void begin();
    void loop();

    void showCoin();
    void showAPQR();
    void showConfigInvalid();
    void showAPIOK();
    void showAPIFailed();

private:
    enum class ShowedScreen {
        NONE,
        AP_QR,
        API_OK,
        API_FAILED,
        CONFIG_INVALID,
        COIN
    };

    Config& m_config;
    TFT_eSPI m_tft;
    TFT_eFEX m_fex;
    TFT_eSprite m_spr;

    ShowedScreen m_lastScreen { ShowedScreen::NONE };
    unsigned long m_lastUpdate { 0 };
    unsigned long m_lastChartUpdate { 0 };
};
