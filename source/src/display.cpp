#include "display.h"
#include "gecko.h"
#include "secrets.h"
#include "settings.h"
#include "utils.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
// https://github.com/peteha99/esp_gen_qr
#include <ESP_QRcode.h>
#include <SPI.h>
#include <locale>
#include <sstream>

#define RGB(r, g, b) (m_tft.color565(r, g, b))

#define CHART_UPDATE_INTERVAL 300000
#define COIN_UPDATE_INTERVAL 10000

Display::Display(Settings& settings)
    : m_settings(settings)
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
    if (m_settings.valid()) {
        showCoin();
    } else {
        showSettingsQR();
    }
}

void Display::showCoin()
{
    if (m_lastScreen == ShowedScreen::COIN
        && (((millis() - m_lastHeartbeat) > 300 && (m_heartbeat == 0 || m_heartbeat == 1 || m_heartbeat == 2))
            || ((millis() - m_lastHeartbeat) > 1500 && (m_heartbeat == 3)))) {
        if (m_heartbeat % 2 == 0) {
            m_fex.drawJpeg("/heart2.jpg", 220, 0);
        } else {
            m_fex.drawJpeg("/heart.jpg", 220, 0);
        }
        m_lastHeartbeat = millis();
        ++m_heartbeat;
        m_heartbeat %= 4;
    }

    if ((millis() - m_lastUpdate) > COIN_UPDATE_INTERVAL
        || !m_settings.displayed()
        || m_lastScreen != ShowedScreen::COIN) {

        double price, price_usd, change;
        bool rewrite(false);
        String msg;

        if (!m_settings.displayed()
            || m_lastScreen != ShowedScreen::COIN) {
            rewrite = true;
        }

        if (m_settings.getGecko().coinPriceChange(m_settings.coin(), m_settings.currency(), price, price_usd, change)) {
            if (rewrite) {
                m_settings.displayed(true);
                m_tft.fillScreen(TFT_BLACK);
                String jpeg("/");
                jpeg += m_settings.symbol() + ".jpg";
                m_fex.drawJpeg(jpeg, 0, 0);
                m_tft.loadFont("NotoSans-Regular30");
                m_tft.setTextColor(TFT_WHITE, TFT_BLACK);
                m_tft.setCursor(60, 0);
                m_tft.println(m_settings.name());
                m_tft.unloadFont();
                m_tft.loadFont("NotoSans-Regular25");
                m_tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
                m_tft.setCursor(60, 30);
                m_tft.println(m_settings.symbol());
                m_tft.unloadFont();
            }

            uint16_t color(change >= 0.0
                    ? RGB(0x10, 0xe0, 0x0)
                    : RGB(0xe0, 0x10, 0x0));

            m_tft.loadFont("NotoSans-Regular40");
            m_tft.setTextColor(color, TFT_BLACK);
            formatNumber(price, msg, m_settings.numberFormat(), false);
            msg += getCurrencySymbol(m_settings.currency().c_str());
            m_tft.setCursor(240 - m_tft.textWidth(msg), 80);
            m_tft.fillRect(0, 80, 240, 40, TFT_BLACK);
            m_tft.println(msg);
            m_tft.unloadFont();

            m_tft.loadFont("NotoSans-Regular25");
            m_tft.setTextColor(color, TFT_BLACK);
            formatNumber(change, msg, m_settings.numberFormat(), true);
            msg += "%";
            auto changeWidth(m_tft.textWidth(msg));
            m_tft.setCursor(240 - changeWidth, 125);
            m_tft.fillRect(0, 125, 240, 25, TFT_BLACK);
            m_tft.println(msg);

            m_tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            formatNumber(price_usd, msg, m_settings.numberFormat(), false);
            msg += "$";
            auto usdWidth(m_tft.textWidth(msg));
            m_tft.setCursor(240 - changeWidth - usdWidth - 15, 125);
            m_tft.println(msg);
            m_tft.unloadFont();

            if ((millis() - m_lastChartUpdate) > CHART_UPDATE_INTERVAL
                || m_lastChartUpdate == 0
                || rewrite) {
                std::vector<double> prices;
                double max, min;
                uint8_t yStart(170);
                uint8_t height(240 - yStart); // 70
                auto calcY = [&max, &min, &height](double price) -> uint8_t {
                    return height - ((height / (max - min)) * (price - min));
                };

                if (m_settings.getGecko().coinChart(m_settings.coin(), m_settings.currency(), prices, max, min)) {
                    m_tft.fillRect(0, yStart - 2, 240, height + 2, TFT_BLACK);

                    // color = TFT_GOLD;
                    for (uint8_t x = 1; x < prices.size(); ++x) {
                        m_tft.drawLine((x - 1) * 10, calcY(prices.at(x - 1)) + yStart, x * 10, calcY(prices.at(x)) + yStart, color);
                        m_tft.drawLine(((x - 1) * 10) + 1, calcY(prices.at(x - 1)) + yStart, (x * 10) + 1, calcY(prices.at(x)) + yStart, color);
                        m_tft.drawLine((x - 1) * 10, calcY(prices.at(x - 1)) + yStart - 1, x * 10, calcY(prices.at(x)) + yStart - 1, color);
                        m_tft.drawLine((x - 1) * 10, calcY(prices.at(x - 1)) + yStart - 2, x * 10, calcY(prices.at(x)) + yStart - 2, color);
                    }

                    m_lastChartUpdate = millis();
                } else {
                    m_tft.loadFont("NotoSans-Regular20");
                    m_tft.setTextColor(TFT_ORANGE, TFT_BLACK);
                    m_tft.setCursor(10, 185);
                    m_tft.println("Chart update failed!");
                    m_tft.unloadFont();

                    m_lastChartUpdate = 0;
                }
            }
            m_lastUpdate = millis();
        }

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

        m_tft.loadFont("NotoSans-Bold20");
        m_tft.setTextColor(TFT_RED, TFT_WHITE);
        constexpr const char* msg = "WiFi Connect:";

        m_tft.setCursor((240 - m_tft.textWidth(msg)) / 2, 5);
        m_tft.println(msg);
        m_tft.unloadFont();

        m_lastScreen = ShowedScreen::AP_QR;
    }
}

void Display::showSettingsQR()
{
    if (m_lastScreen != ShowedScreen::SETTINGS_QR) {
        m_tft.fillScreen(TFT_WHITE);

        std::string url("http://");
        url += WiFi.localIP().toString().c_str();
        url += "/";
        ESP_QRcode tftQR(&m_tft);
        tftQR.qrcode(url.c_str(), 20, 40, 200, 3);

        m_tft.loadFont("NotoSans-Bold30");
        m_tft.setCursor(5, 5);
        m_tft.setTextColor(TFT_RED, TFT_WHITE);
        m_tft.println("Open Settings:");
        m_tft.unloadFont();

        m_tft.loadFont("NotoSans-Regular25");
        m_tft.setCursor(5, 30);
        m_tft.println(url.c_str());
        m_tft.unloadFont();

        m_lastScreen = ShowedScreen::SETTINGS_QR;
    }
}

void Display::showAPIOK()
{
    if (m_lastScreen != ShowedScreen::API_OK) {
        m_tft.loadFont("NotoSans-Bold30");
        m_tft.fillScreen(TFT_SKYBLUE);
        constexpr const char* msg = "To The Moon!";
        m_tft.setCursor((240 - m_tft.textWidth(msg)) / 2, 95);
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
        m_tft.loadFont("NotoSans-Bold30");
        m_tft.fillScreen(TFT_RED);
        constexpr const char* msg = "Gecko API failed!";
        m_tft.setCursor((240 - m_tft.textWidth(msg)) / 2, 95);
        m_tft.setTextColor(TFT_BLACK, TFT_RED);
        m_tft.println(msg);
        m_tft.unloadFont();

        Serial.println(msg);
        m_lastScreen = ShowedScreen::API_FAILED;
    }
}
