#include "display.h"
#include "config.h"
#include "gecko.h"
#include "secrets.h"
#include "utils.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
// https://github.com/peteha99/esp_gen_qr
#include <ESP_QRcode.h>
#include <SPI.h>
#include <locale>
#include <sstream>

#define RGB(r, g, b) (m_tft.color565(r, g, b))

Display::Display(Config& config)
    : m_config(config)
    , m_tft(TFT_eSPI())
    , m_fex(TFT_eFEX(&m_tft))
    , m_spr(TFT_eSprite(&m_tft))
{
}

void Display::begin()
{
    m_tft.begin();
    m_tft.setRotation(2); // 0 & 2 Portrait. 1 & 3 landscape
    m_spr.createSprite(100, 20);
    m_fex.listSPIFFS(); // Lists the files so you can see what is in the SPIFFS}

    showAPQR();
}

void Display::loop()
{
    if (m_config.valid()) {
        showCoin();
    } else {
        showConfigInvalid();
    }
}

void Display::showCoin()
{
    if ((millis() - m_lastUpdate) > 1000 || m_lastScreen != ShowedScreen::COIN) {
        if (m_lastScreen != ShowedScreen::COIN) {
            m_tft.fillScreen(TFT_BLACK);
        }

        m_tft.loadFont("Roboto-Regular14");
        m_tft.setTextColor(TFT_WHITE, TFT_BLACK);
        String msg(millis());
        m_tft.fillRect(0, 220, 240, 14, TFT_BLACK);
        m_tft.setCursor(m_tft.width() - m_tft.textWidth(msg), 220);
        m_tft.println(msg);
        m_tft.unloadFont();

        double price, change;
        if (m_config.getGecko().coinPriceChange(m_config.coin(), m_config.currency(), price, change)) {
            m_tft.loadFont("Roboto-Medium40");
            m_tft.setTextColor(change >= 0.0
                    ? RGB(0x10, 0xe0, 0x0)
                    : RGB(0xe0, 0x10, 0x0),
                TFT_BLACK);

            msg = String(price) + " " + getCurrencySymbol(m_config.currency().c_str());

            m_tft.setCursor(m_tft.width() - m_tft.textWidth(msg), 110);
            m_tft.println(msg);
            m_tft.unloadFont();
        }

        m_lastUpdate = millis();
        m_lastScreen = ShowedScreen::COIN;
    }
}

void Display::showAPQR()
{
    if (m_lastScreen != ShowedScreen::AP_QR) {
        m_tft.fillScreen(TFT_WHITE);

        std::string qrText;
        qrText = "WIFI:T:WPA;S:";
        qrText += hostname;
        qrText += ";P:";
        qrText += ap_password;
        qrText += ";H:;";
        ESP_QRcode tftQR(&m_tft);
        tftQR.qrcode(qrText.c_str(), 20, 40, 200, 3);

        m_tft.loadFont("Roboto-Bold30");
        m_tft.setTextColor(TFT_RED, TFT_WHITE);
        constexpr const char* msg = "WiFi Connect:";

        m_tft.setCursor((m_tft.width() - m_tft.textWidth(msg)) / 2, 5);
        m_tft.println(msg);
        m_tft.unloadFont();

        m_lastScreen = ShowedScreen::AP_QR;
    }
}

void Display::showConfigInvalid()
{
    if (m_lastScreen != ShowedScreen::CONFIG_INVALID) {
        m_tft.fillScreen(TFT_WHITE);

        std::string url("http://");
        url += WiFi.localIP().toString().c_str();
        url += "/config";
        ESP_QRcode tftQR(&m_tft);
        tftQR.qrcode(url.c_str(), 20, 40, 200, 3);

        m_tft.loadFont("Roboto-Bold20");
        m_tft.setCursor(5, 5);
        m_tft.setTextColor(TFT_RED, TFT_WHITE);
        m_tft.println("Open Configuration:");
        m_tft.unloadFont();

        m_tft.loadFont("Roboto-Medium16");
        m_tft.setCursor(5, 30);
        m_tft.println(url.c_str());
        m_tft.unloadFont();

        m_lastScreen = ShowedScreen::CONFIG_INVALID;
    }
}

void Display::showAPIOK()
{
    if (m_lastScreen != ShowedScreen::API_OK) {
        m_tft.loadFont("Roboto-Bold30");
        m_tft.fillScreen(TFT_SKYBLUE);
        constexpr const char* msg = "To The Moon!";
        m_tft.setCursor((m_tft.width() - m_tft.textWidth(msg)) / 2, 95);
        m_tft.setTextColor(TFT_WHITE, TFT_SKYBLUE);
        m_tft.println(msg);
        m_tft.unloadFont();

        Serial.println(msg);
        m_lastScreen = ShowedScreen::API_OK;
    }
}

void Display::showAPIFailed()
{
    if (m_lastScreen != ShowedScreen::API_FAILED) {
        m_tft.loadFont("Roboto-Bold30");
        m_tft.fillScreen(TFT_RED);
        constexpr const char* msg = "Gecko API failed!";
        m_tft.setCursor((m_tft.width() - m_tft.textWidth(msg)) / 2, 95);
        m_tft.setTextColor(TFT_BLACK, TFT_RED);
        m_tft.println(msg);
        m_tft.unloadFont();

        Serial.println(msg);
        m_lastScreen = ShowedScreen::API_FAILED;
    }
}
