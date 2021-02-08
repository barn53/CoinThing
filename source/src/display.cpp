#include "display.h"
#include "gecko.h"
#include "pre.h"
#include "settings.h"
#include <Arduino.h>
#include <ESP_QRcode.h> // https://github.com/peteha99/esp_gen_qr
#include <SPI.h>
#include <locale>
#include <sstream>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define RGB(r, g, b) (m_tft.color565(r, g, b))

#define Red565 RGB(0x10, 0xd0, 0x0)
#define Green565 RGB(0xd0, 0x10, 0x0)

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define SPRITE_WIDTH 30
#define SPRITE_PRICE_HEIGHT 45
#define SPRITE_CHANGE_HEIGHT 23
#define DISTANCE_CHART_VALUE 3
#define HEIGHT_CHART_VALUE 6

#define CHART_UPDATE_INTERVAL (5 * 1000)

Display::Display(Gecko& gecko, const Settings& settings)
    : m_gecko(gecko)
    , m_settings(settings)
    , m_tft(TFT_eSPI())
    , m_fex(TFT_eFEX(&m_tft))
{
}

void Display::begin()
{
    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 30);

    m_tft.begin();
    m_tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
#if COIN_THING_SERIAL == 1
    m_fex.listSPIFFS(); // Lists the files so you can see what is in the SPIFFS}
#endif

    analogWriteRange(std::numeric_limits<uint8_t>::max());
    analogWrite(TFT_BL, std::numeric_limits<uint8_t>::max());

    pinMode(NEO_AND, OUTPUT);
    digitalWrite(NEO_AND, 1);

    m_neo.Begin();
    m_neo.SetPixelColor(0, RgbColor(0xff, 0x00, 0x10));
    m_neo.Show();
}

void Display::loop()
{
    m_gecko.loop();
    analogWrite(TFT_BL, m_settings.brightness());
    if (m_settings.valid()) {
        showCoin();
    } else {
        showSettingsQR();
    }
}

void Display::heartbeat()
{
    if (!m_settings.heartbeat()) {
        m_last_heartbeat = 0;
        m_heart_beat_count = 0;
        return;
    }

    if (m_last_screen == Screen::COIN
        && (((millis_test() - m_last_heartbeat) > 300
                && (m_heart_beat_count == 0 || m_heart_beat_count == 1 || m_heart_beat_count == 2))
            || ((millis_test() - m_last_heartbeat) > 1500
                && (m_heart_beat_count == 3)))) {
        if (m_heart_beat_count % 2 == 0) {
            m_fex.drawJpeg("/heart2.jpg", 220, 0);
        } else {
            m_fex.drawJpeg("/heart.jpg", 220, 0);
        }
        m_last_heartbeat = millis_test();
        ++m_heart_beat_count;
        m_heart_beat_count %= 4;
    }
}

void Display::renderTitle()
{
    LOG_FUNC

    m_tft.fillScreen(TFT_BLACK);
    m_tft.setTextWrap(false);
    String symbol("/");
    symbol += m_settings.symbol();
    symbol += ".jpg";
    int16_t x_name(0);
    if (SPIFFS.exists(symbol)) {
        m_fex.drawJpeg(symbol, 0, 0);
        x_name = 60;
    }
    m_tft.loadFont("NotoSans-Regular30");
    m_tft.setTextColor(TFT_WHITE, TFT_BLACK);
    m_tft.setCursor(x_name, 0);
    m_tft.println(m_settings.name());
    m_tft.unloadFont();
    m_tft.loadFont("NotoSans-Regular25");
    m_tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    m_tft.setCursor(x_name, 30);
    m_tft.print(m_settings.symbol());
    m_tft.print(" - ");
    String upperCurrency(m_settings.currency());
    upperCurrency.toUpperCase();
    m_tft.println(upperCurrency);
    m_tft.unloadFont();
}

void Display::renderCoin()
{
    LOG_FUNC

    uint16_t color(m_gecko.change_pct() >= 0.0
            ? Red565
            : Green565);

    String msg;
    TFT_eSprite sprite(&m_tft);
    sprite.setColorDepth(8);
    sprite.createSprite(SPRITE_WIDTH, SPRITE_PRICE_HEIGHT);
    sprite.setTextColor(color, TFT_BLACK);

    sprite.loadFont("NotoSans-Regular50");
    sprite.setTextWrap(false);
    formatNumber(m_gecko.price(), msg, m_settings.numberFormat(), false, true);
    msg += getCurrencySymbol(m_settings.currency());

    auto priceWidth(sprite.textWidth(msg));
    if (priceWidth > DISPLAY_WIDTH) {
        sprite.unloadFont();
        sprite.loadFont("NotoSans-Condensed50");
        priceWidth = sprite.textWidth(msg);
        if (priceWidth > DISPLAY_WIDTH) {
            sprite.unloadFont();
            sprite.loadFont("NotoSans-ExtraCondensed50");
            priceWidth = sprite.textWidth(msg);
        }
    }

    for (uint8_t page = 0; page < (DISPLAY_WIDTH / SPRITE_WIDTH); ++page) {
        sprite.fillSprite(TFT_BLACK);
        auto x((DISPLAY_WIDTH - priceWidth) - (page * SPRITE_WIDTH));
        sprite.setCursor(x, 0);
        sprite.println(msg);

        sprite.pushSprite(page * SPRITE_WIDTH, 70);
    }
    sprite.unloadFont();
    sprite.deleteSprite();

    sprite.loadFont("NotoSans-Regular25");
    sprite.createSprite(SPRITE_WIDTH, SPRITE_CHANGE_HEIGHT);

    String usdMsg;
    formatNumber(m_gecko.price_usd(), usdMsg, m_settings.numberFormat(), false, true);
    formatNumber(m_gecko.change_pct(), msg, m_settings.numberFormat(), true, false, 2);
    usdMsg += "$";
    msg += "%";
    auto usdWidth(sprite.textWidth(usdMsg));
    auto changeWidth(sprite.textWidth(msg));

    if ((usdWidth + changeWidth + 15) > DISPLAY_WIDTH) {
        sprite.unloadFont();
        sprite.loadFont("NotoSans-Condensed25");
        usdWidth = sprite.textWidth(usdMsg);
        changeWidth = sprite.textWidth(msg);
    }

    for (uint8_t page = 0; page < (DISPLAY_WIDTH / SPRITE_WIDTH); ++page) {
        sprite.fillSprite(TFT_BLACK);

        auto x((DISPLAY_WIDTH - usdWidth - changeWidth - 15) - (page * SPRITE_WIDTH));
        sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
        sprite.setCursor(x, 0);
        sprite.println(usdMsg);

        x = (DISPLAY_WIDTH - changeWidth) - (page * SPRITE_WIDTH);
        sprite.setTextColor(color, TFT_BLACK);
        sprite.setCursor(x, 0);
        sprite.println(msg);

        sprite.pushSprite(page * SPRITE_WIDTH, 125);
    }

    sprite.unloadFont();
    sprite.deleteSprite();

    m_last_price_update = millis_test();
}

bool Display::renderChart(Settings::Chart type)
{
    LOG_FUNC

    const std::vector<gecko_t>* prices(nullptr);

    if (type == Settings::Chart::CHART_24_H) {
        prices = &m_gecko.chart_48h();
    } else if (type == Settings::Chart::CHART_48_H) {
        prices = &m_gecko.chart_48h();
    } else if (type == Settings::Chart::CHART_30_D) {
        prices = &m_gecko.chart_30d();
    } else {
        return false;
    }

    if (prices->empty()) {
        return false;
    }

    gecko_t max = *(std::max_element(prices->begin(), prices->end()));
    gecko_t min = *(std::min_element(prices->begin(), prices->end()));

    uint16_t color(prices->front() > prices->back()
            ? Red565
            : Green565);

    auto maxLabelColor(TFT_GOLD);

    uint8_t yStart(170);
    uint8_t height(DISPLAY_HEIGHT - yStart); // 70
    m_tft.fillRect(0, yStart - HEIGHT_CHART_VALUE, DISPLAY_WIDTH, height + HEIGHT_CHART_VALUE, TFT_BLACK);

    uint8_t xPerVaue(DISPLAY_WIDTH / prices->size());
    uint8_t xAtMax(0);
    for (uint8_t x = 1; x < prices->size(); ++x) {
        if (prices->at(x) >= max) {
            xAtMax = x * xPerVaue;
        }
    }

    m_tft.loadFont("NotoSans-Regular13");
    String maxMsg;
    formatNumber(max, maxMsg, m_settings.numberFormat(), false, true);
    auto maxWidth(m_tft.textWidth(maxMsg));
    uint8_t lineXLeft(0);
    uint8_t lineXRight(0);
    uint8_t maxLabelX;
    uint8_t maxLabelY(yStart - 2 - (HEIGHT_CHART_VALUE / 2));
    bool maxLabelLeft(true);

    if (xAtMax > DISPLAY_WIDTH / 2) { // at the left display side
        lineXLeft = maxWidth + (2 * DISTANCE_CHART_VALUE);
        lineXRight = xAtMax - DISTANCE_CHART_VALUE;

        // m_tft.drawLine(maxWidth + DISTANCE_CHART_VALUE, (yStart - HEIGHT_CHART_VALUE), lineXLeft, yStart, maxLabelColor);
        // m_tft.drawLine(maxWidth + DISTANCE_CHART_VALUE, (yStart + HEIGHT_CHART_VALUE), lineXLeft, yStart, maxLabelColor);

        maxLabelX = 0;
    } else { // at the right display side
        maxLabelLeft = false;
        lineXLeft = xAtMax + DISTANCE_CHART_VALUE;
        lineXRight = DISPLAY_WIDTH - maxWidth - (2 * DISTANCE_CHART_VALUE);

        // m_tft.drawLine(DISPLAY_WIDTH - maxWidth - DISTANCE_CHART_VALUE, (yStart - HEIGHT_CHART_VALUE), lineXRight, yStart, maxLabelColor);
        // m_tft.drawLine(DISPLAY_WIDTH - maxWidth - DISTANCE_CHART_VALUE, (yStart + HEIGHT_CHART_VALUE), lineXRight, yStart, maxLabelColor);

        maxLabelX = DISPLAY_WIDTH - maxWidth;
    }

    auto calcY = [&max, &min, &height](gecko_t price) -> uint8_t {
        return height - ((height / (max - min)) * (price - min));
    };

    uint8_t dotted(0);
    for (uint8_t x = lineXLeft; x < lineXRight; ++x) {
        if (dotted >= 0 && dotted < 5) {
            m_tft.drawPixel(x, yStart, maxLabelColor);
        }
        ++dotted;
        dotted %= 9;
    }

    for (uint8_t x = 1; x < prices->size(); ++x) {
        m_tft.drawLine((x - 1) * xPerVaue, calcY(prices->at(x - 1)) + yStart, x * xPerVaue, calcY(prices->at(x)) + yStart, color);
        m_tft.drawLine(((x - 1) * xPerVaue) + 1, calcY(prices->at(x - 1)) + yStart, (x * xPerVaue) + 1, calcY(prices->at(x)) + yStart, color);
        m_tft.drawLine((x - 1) * xPerVaue, calcY(prices->at(x - 1)) + yStart - 1, x * xPerVaue, calcY(prices->at(x)) + yStart - 1, color);
        m_tft.drawLine((x - 1) * xPerVaue, calcY(prices->at(x - 1)) + yStart - 2, x * xPerVaue, calcY(prices->at(x)) + yStart - 2, color);
    }

    String period("24h");
    if (type == Settings::Chart::CHART_30_D) {
        period = "30d";
    }
    auto widthPeriod(m_tft.textWidth(period));
    uint8_t periodX(DISTANCE_CHART_VALUE);
    uint8_t periodY(0);
    uint8_t yPrice(0);

    if (maxLabelLeft) { // then period label right
        periodX = DISPLAY_WIDTH - widthPeriod - DISTANCE_CHART_VALUE;
        yPrice = calcY(prices->back());
    } else {
        yPrice = calcY(prices->front());
    }

    if (yPrice < (height / 2)) {
        periodY = DISPLAY_HEIGHT - 12;
    } else {
        periodY = yStart;
    }

    m_tft.fillRect(periodX, periodY, widthPeriod, 2 * HEIGHT_CHART_VALUE, TFT_BLACK);
    m_tft.setTextColor(TFT_PURPLE, TFT_BLACK);
    m_tft.setCursor(periodX, periodY);
    m_tft.println(period);

    m_tft.fillRect(maxLabelX, yStart - HEIGHT_CHART_VALUE, maxWidth, DISTANCE_CHART_VALUE + (2 * HEIGHT_CHART_VALUE), TFT_BLACK);
    m_tft.setTextColor(maxLabelColor, TFT_BLACK);
    m_tft.setCursor(maxLabelX, maxLabelY);
    m_tft.println(maxMsg);
    m_tft.unloadFont();

    m_last_chart_update = millis_test();
    return true;
}

void Display::chartFailed()
{
    m_tft.loadFont("NotoSans-Regular20");
    m_tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    m_tft.setCursor(10, 185);
    m_tft.println("Chart update failed!");
    m_tft.unloadFont();
    m_last_chart_update = 0;
}

Settings::Chart Display::nextChartType()
{
    Settings::Chart next(m_last_chart);

    if (m_last_chart == Settings::Chart::CHART_24_H) {
        if (m_settings.chart() & Settings::Chart::CHART_48_H) {
            next = Settings::Chart::CHART_48_H;
        } else if (m_settings.chart() & Settings::Chart::CHART_30_D) {
            next = Settings::Chart::CHART_30_D;
        }
    } else if (m_last_chart == Settings::Chart::CHART_48_H) {
        if (m_settings.chart() & Settings::Chart::CHART_30_D) {
            next = Settings::Chart::CHART_30_D;
        } else if (m_settings.chart() & Settings::Chart::CHART_48_H) {
            next = Settings::Chart::CHART_48_H;
        }
    } else if (m_last_chart == Settings::Chart::CHART_30_D) {
        if (m_settings.chart() & Settings::Chart::CHART_24_H) {
            next = Settings::Chart::CHART_24_H;
        } else if (m_settings.chart() & Settings::Chart::CHART_48_H) {
            next = Settings::Chart::CHART_48_H;
        }
    }

    return next;
}

void Display::showCoin()
{
    bool rewrite(false);

    if (m_last_screen != Screen::COIN
        || doChange(m_settings.lastChange(), m_last_seen_settings)) {
        renderTitle();
        rewrite = true;
        m_last_seen_settings = millis_test();
    }
    heartbeat();

    if (rewrite
        || doChange(m_gecko.lastPriceFetch(), m_last_price_update)) {
        renderCoin();
    }

    if (rewrite
        || doInterval(m_last_chart_update, CHART_UPDATE_INTERVAL)) {
        Settings::Chart next(nextChartType());
#if COIN_THING_SERIAL > 0
        Serial.printf("last type: %u -> next type: %u, setting: %u\n", m_last_chart, next, m_settings.chart());
#endif
        if (!renderChart(next)) {
            chartFailed();
        }
        m_last_chart = next;
    }

    m_last_screen = Screen::COIN;
}

void Display::showAPQR()
{
    if (m_last_screen != Screen::AP_QR) {
        m_tft.fillScreen(TFT_WHITE);

        std::string qrText;
        qrText = "WIFI:T:WPA;S:";
        qrText += HOST_NAME;
        qrText += ";P:";
        qrText += SECRET_AP_PASSWORD;
        qrText += ";H:;";
        ESP_QRcode tftQR(&m_tft);
        tftQR.qrcode(qrText.c_str(), 20, 40, 200, 3);

        m_tft.loadFont("NotoSans-Regular20");
        m_tft.setTextColor(TFT_RED, TFT_WHITE);
        constexpr const char* msg = "WiFi Connect:";

        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 5);
        m_tft.println(msg);
        m_tft.unloadFont();

        m_tft.loadFont("NotoSans-Regular15");
        m_tft.println(qrText.c_str());
        m_tft.unloadFont();

        m_last_screen = Screen::AP_QR;
    }
}

void Display::showSettingsQR()
{
    if (m_last_screen != Screen::SETTINGS_QR) {
        m_tft.fillScreen(TFT_WHITE);

        std::string url("http://");
        url += WiFi.localIP().toString().c_str();
        url += "/";
        ESP_QRcode tftQR(&m_tft);
        tftQR.qrcode(url.c_str(), 20, 40, 200, 3);

        m_tft.loadFont("NotoSans-Regular20");
        m_tft.setCursor(5, 5);
        m_tft.setTextColor(TFT_RED, TFT_WHITE);
        m_tft.println("Open Settings:");
        m_tft.unloadFont();

        m_tft.loadFont("NotoSans-Regular15");
        m_tft.setCursor(5, 30);
        m_tft.println(url.c_str());
        m_tft.unloadFont();

        m_last_screen = Screen::SETTINGS_QR;
    }
}

void Display::showAPIOK()
{
    if (m_last_screen != Screen::API_OK) {
        m_tft.loadFont("NotoSans-Regular30");
        m_tft.fillScreen(TFT_SKYBLUE);
        constexpr const char* msg = "To The Moon!";
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 95);
        m_tft.setTextColor(TFT_WHITE, TFT_SKYBLUE);
        m_tft.println(msg);
        m_tft.unloadFont();
        m_last_screen = Screen::API_OK;
    }
}

void Display::showAPIFailed()
{
    if (m_last_screen != Screen::API_FAILED) {
        m_tft.loadFont("NotoSans-Regular30");
        m_tft.fillScreen(TFT_RED);
        constexpr const char* msg = "Gecko API failed!";
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 95);
        m_tft.setTextColor(TFT_BLACK, TFT_RED);
        m_tft.println(msg);
        m_tft.unloadFont();
        m_last_screen = Screen::API_FAILED;
    }
}
