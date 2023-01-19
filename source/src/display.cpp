#include "display.h"
#include "common.h"
#include "gecko.h"
#include "pre.h"
#include "settings.h"
#include <ESP_QRcode.h>
#include <SPI.h>
#include <locale>
#include <sstream>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define RGB(r, g, b) (xTft.color565(r, g, b))

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240

#define CHART_Y_START 160
#define CHART_HEIGHT (DISPLAY_HEIGHT - CHART_Y_START)
#define CHART_MIDDLE (CHART_Y_START + (CHART_HEIGHT / 2))
#define DISTANCE_CHART_VALUE 3
#define POWERUP_SEQUENCE_INDICATOR_Y 120

#define API_OK_SHOW_TIME (4 * 1000)
#define ON_SCREEN_DEBUG_INTERVAL (5 * 1000)

extern String xHostname;
extern Settings xSettings;
extern Gecko xGecko;
extern TFT_eSPI xTft;

uint16_t Display::RED565;
uint16_t Display::GREEN565;
uint16_t Display::GREY_LEVEL2;
uint16_t Display::PERIOD_COLOR;
uint16_t Display::CURRENT_COIN_DOT_COLOR;
uint16_t Display::CHART_VERTICAL_LINE_COLOR;
uint16_t Display::CHART_FIRST_COLOR;
uint16_t Display::CHART_LAST_COLOR;
uint16_t Display::CHART_HIGH_COLOR;
uint16_t Display::CHART_LOW_COLOR;
uint16_t Display::CHART_BOX_BG;
uint16_t Display::CHART_BOX_MARGIN_COLOR;

Display::Display()
{
    RED565 = RGB(0xd8, 0x0, 0x0); // -> rgb565: 0xd800
    GREEN565 = RGB(0x0, 0xd8, 0x0); // -> rgb565: 0x06c0
    GREY_LEVEL2 = TFT_DARKGREY;
    PERIOD_COLOR = RGB(0x0, 0x30, 0x90);
    CURRENT_COIN_DOT_COLOR = RGB(0xff, 0x66, 0x0);
    CHART_VERTICAL_LINE_COLOR = RGB(0x03, 0x04, 0x03);
    CHART_FIRST_COLOR = RGB(0xa0, 0x0, 0xa0);
    CHART_LAST_COLOR = RGB(0xa0, 0x80, 0x0);
    CHART_HIGH_COLOR = TFT_DARKGREEN;
    CHART_LOW_COLOR = TFT_RED;
    CHART_BOX_BG = RGB(0x03, 0x03, 0x03);
    CHART_BOX_MARGIN_COLOR = RGB(0x20, 0x20, 0x20);
}

void Display::begin(uint8_t powerupSequenceCounter)
{
    analogWriteRange(std::numeric_limits<uint8_t>::max());
    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, std::numeric_limits<uint8_t>::max());

    uint8_t colorSet = xSettings.getColorSet();
    if (colorSet == 1) {
        RED565 = RGB(0xee, 0x0, 0xa);
        GREEN565 = RGB(0x0, 0xee, 0xa);
        GREY_LEVEL2 = RGB(0xb8, 0xb8, 0xb8);
        PERIOD_COLOR = RGB(0x0, 0x7a, 0xea);
        CURRENT_COIN_DOT_COLOR = RGB(0xdd, 0xaa, 0x0);
        CHART_VERTICAL_LINE_COLOR = RGB(0x30, 0x30, 0x30);
        CHART_FIRST_COLOR = RGB(0xa0, 0x0, 0xa0);
        CHART_LAST_COLOR = RGB(0xa0, 0x80, 0x0);
        CHART_HIGH_COLOR = RGB(0x0, 0xee, 0xa);
        CHART_LOW_COLOR = RGB(0xee, 0x0, 0xa);
        CHART_BOX_BG = RGB(0x03, 0x03, 0x03);
        CHART_BOX_MARGIN_COLOR = RGB(0x20, 0x20, 0x20);
    }

    xTft.begin();
    xTft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
    xTft.setTextWrap(false);
    xTft.fillScreen(TFT_BLACK);
    xTft.setTextColor(TFT_WHITE, TFT_BLACK);

    String msg;
    showCoinThing(msg, 30);
    if (powerupSequenceCounter >= POWERUP_SEQUENCE_COUNT_TO_RESET) {
        xTft.loadFont(F("NotoSans-Regular30"));
        xTft.setTextColor(TFT_RED, TFT_BLACK);
        msg = F("Factory reset");
        xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 180);
        xTft.print(msg);
        xTft.unloadFont();
    }

    if (powerupSequenceCounter > 1) {
        for (uint8_t ii = 1; ii <= POWERUP_SEQUENCE_COUNT_TO_RESET; ++ii) {
            uint16_t color(GREY_LEVEL2);
            if (ii < POWERUP_SEQUENCE_COUNT_TO_RESET) {
                if (ii <= powerupSequenceCounter) {
                    color = TFT_RED;
                }
                xTft.fillCircle(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20)), POWERUP_SEQUENCE_INDICATOR_Y, 5, color);
            } else {
                if (powerupSequenceCounter >= POWERUP_SEQUENCE_COUNT_TO_RESET) {
                    color = TFT_RED;
                }
                xTft.drawLine(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) - 5), POWERUP_SEQUENCE_INDICATOR_Y - 5, ((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) + 5), POWERUP_SEQUENCE_INDICATOR_Y + 5, color);
                xTft.drawLine(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) - 4), POWERUP_SEQUENCE_INDICATOR_Y - 5, ((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) + 6), POWERUP_SEQUENCE_INDICATOR_Y + 5, color);
                xTft.drawLine(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) - 5), POWERUP_SEQUENCE_INDICATOR_Y - 4, ((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) + 5), POWERUP_SEQUENCE_INDICATOR_Y + 6, color);
                xTft.drawLine(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) - 4), POWERUP_SEQUENCE_INDICATOR_Y - 4, ((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) + 6), POWERUP_SEQUENCE_INDICATOR_Y + 6, color);

                xTft.drawLine(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) + 5), POWERUP_SEQUENCE_INDICATOR_Y - 5, ((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) - 5), POWERUP_SEQUENCE_INDICATOR_Y + 5, color);
                xTft.drawLine(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) + 6), POWERUP_SEQUENCE_INDICATOR_Y - 5, ((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) - 4), POWERUP_SEQUENCE_INDICATOR_Y + 5, color);
                xTft.drawLine(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) + 5), POWERUP_SEQUENCE_INDICATOR_Y - 4, ((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) - 5), POWERUP_SEQUENCE_INDICATOR_Y + 6, color);
                xTft.drawLine(((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) + 6), POWERUP_SEQUENCE_INDICATOR_Y - 4, ((DISPLAY_WIDTH / 2) - (POWERUP_SEQUENCE_COUNT_TO_RESET * 20 / 2) + (ii * 20) - 4), POWERUP_SEQUENCE_INDICATOR_Y + 6, color);
            }
        }
    }
}

void Display::loop()
{
    LOG_FUNC

    analogWrite(TFT_BL, xSettings.brightness());

    xGecko.loop();
    if (xGecko.succeeded()) {
        if (m_show_api_ok) {
            if (m_display_start == 0) {
                m_display_start = millis_test();
            }
            if (millis_test() - m_display_start < API_OK_SHOW_TIME) {
                showAPIOK();
            } else {
                m_show_api_ok = false;
            }
        } else {
            if (xSettings.valid()) {
                uint8_t pct(xGecko.recoverFromHTTP429());
                if (pct < 100) {
                    showRecover(pct);
                    m_previous_prefetch_failed = false;
                } else {
                    if (xSettings.mode() == Settings::Mode::ONE_COIN) {
                        showCoin();
                    } else if (xSettings.mode() == Settings::Mode::TWO_COINS) {
                        showTwoCoins();
                    } else if (xSettings.mode() == Settings::Mode::MULTIPLE_COINS) {
                        showMultipleCoins();
                    }
                    m_shows_recover = false;
                }
            } else {
                showSettingsQR();
            }
        }
        if (m_on_screen_debug_enabled) {
            showOnScreenDebug();
        }
    } else {
        showAPIFailed();
    }
}

void Display::wifiConnect()
{
    uint8_t yWiFi(0);
    if (xSettings.heartbeat()) {
        yWiFi = 25;
    }

    if (WiFi.isConnected()) {
        if (m_shows_wifi_not_connected) {
            xTft.fillRect(215, yWiFi, 25, 25, TFT_BLACK);
            m_shows_wifi_not_connected = false;
        }
    } else {
        if (!m_shows_wifi_not_connected) {
            drawBmp(F("/nowifi.bmp"), xTft, 215, yWiFi);
            m_shows_wifi_not_connected = true;
        }
    }
}

void Display::showRecover(uint8_t pct)
{
    if (!m_shows_recover) {
        xTft.fillRect(210, 0, 30, 28, TFT_BLACK); // clear
        xTft.fillRect(224, 2, 4, 16, TFT_WHITE); // pause icon
        xTft.fillRect(233, 2, 4, 16, TFT_WHITE); // pause icon
        xTft.fillRect(220, 25, 20, 3, CURRENT_COIN_DOT_COLOR); // 100% bar
    }
    xTft.fillRect(220, 25, (2000 / 100 * pct) / 100, 3, TFT_BLACK); // shrink bar to pct
    m_shows_recover = true;
}

void Display::heartbeat()
{
    if (!xSettings.heartbeat()) {
        m_last_heartbeat = 0;
        m_heart_beat_count = 0;
        return;
    }

    if (m_last_screen == Screen::COIN
        && (((millis_test() - m_last_heartbeat) > 300
                && (m_heart_beat_count == 0
                    || m_heart_beat_count == 1
                    || m_heart_beat_count == 2))
            || ((millis_test() - m_last_heartbeat) > 1500
                && (m_heart_beat_count == 3)))) {
        if (m_heart_beat_count % 2 == 0) {
            drawBmp(F("/heart2.bmp"), xTft, 220, 0);
        } else {
            drawBmp(F("/heart.bmp"), xTft, 220, 0);
        }
        m_last_heartbeat = millis_test();
        ++m_heart_beat_count;
        m_heart_beat_count %= 4;
    }
}

void Display::showOnScreenDebug()
{
    if (doInterval(m_last_on_screen_debug_update, ON_SCREEN_DEBUG_INTERVAL)) {
        xTft.loadFont(F("NotoSans-Regular20"));
        xTft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
        xTft.fillRect(5, 5, 230, 20, TFT_BLACK);
        xTft.setCursor(7, 5);
        xTft.print(F("req:"));
        xTft.print(xGecko.getHttpCount());
        if (xGecko.increaseIntervalDueToHTTP429()) {
            xTft.print(F(" slow"));
            if (xGecko.recoverFromHTTP429() < 100) {
                xTft.print(F(" p "));
                xTft.print(100 - xGecko.recoverFromHTTP429());
            }
        }

        xTft.fillRect(5, 25, 230, 20, TFT_BLACK);
        xTft.setCursor(7, 25);
        xTft.print(F("429:"));
        xTft.print(xGecko.http429Count());

        xTft.print(F(" 429p:"));
        xTft.print(xGecko.http429PauseCount());

        xTft.fillRect(5, 45, 230, 20, TFT_BLACK);
        xTft.setCursor(7, 45);
        xTft.print(F("last429:"));
        if (xGecko.lastHttp429Persist() > 0) {
            xTft.print((millis_test() - xGecko.lastHttp429Persist()) / 1000 / 60);
            xTft.print(F("m"));
        } else {
            xTft.print(F("-"));
        }
        xTft.print(F(" run:"));
        xTft.print(millis_test() / 1000 / 60);
        xTft.print(F("m"));

        m_last_on_screen_debug_update = millis_test();
    }
}

void Display::renderTitle()
{
    LOG_FUNC

    String name;
    String symbol;
    String currency;

    xTft.fillScreen(TFT_BLACK);
    m_shows_wifi_not_connected = false;
    m_last_on_screen_debug_update = false;

    if (xSettings.mode() != Settings::Mode::TWO_COINS) {
        name = xSettings.name(m_current_coin_index);
        symbol = xSettings.symbol(m_current_coin_index);
        currency = xSettings.currency();

        int16_t x_name(0);
        String icon(F("/"));
        icon += symbol;
        icon += F(".bmp");
        if (drawBmp(icon, xTft, 0, 0)) {
            x_name = 60;
        }
        uint8_t y_symbol_curr(35);
        if (xSettings.mode() == Settings::Mode::MULTIPLE_COINS) {
            xTft.loadFont(F("NotoSans-Regular25"));
            y_symbol_curr = 32;
        } else {
            xTft.loadFont(F("NotoSans-Regular25"));
        }
        xTft.setTextColor(GREY_LEVEL2, TFT_BLACK);
        xTft.setCursor(x_name, y_symbol_curr);
        xTft.print(symbol);
        xTft.print(" - ");
        xTft.print(currency);
        xTft.unloadFont();

        // Write the name at last, so it appears on top of the symbol and currency
        //  when there is a collision
        xTft.loadFont(F("NotoSans-Regular30"));
        if (xTft.textWidth(name) > DISPLAY_WIDTH - x_name) {
            xTft.unloadFont();
            xTft.loadFont(F("NotoSans-Condensed30"));
        }
        xTft.setTextColor(TFT_WHITE, TFT_BLACK);
        xTft.setCursor(x_name, 0);
        xTft.print(name);
        xTft.unloadFont();

        if (xSettings.mode() == Settings::Mode::MULTIPLE_COINS) {
            uint32_t count(0);
            uint16_t color(GREY_LEVEL2);
            for (uint32_t xx = x_name + 4; count < xSettings.numberCoins(); ++count, xx += 13) {
                if (count == m_current_coin_index) {
                    color = CURRENT_COIN_DOT_COLOR;
                    for (uint8_t rr = 0; rr < 2; ++rr) {
                        xTft.fillCircle(xx, 60, 2, color);
                        xx += 3;
                    }
                } else {
                    color = GREY_LEVEL2;
                }
                xTft.fillCircle(xx, 60, 2, color);
            }
        }
    } else { // Settings::Mode::TWO_COINS
        for (auto ii : { 0, 1 }) {
            int16_t y_cursor(ii * 127);
            name = xSettings.name(ii);
            symbol = xSettings.symbol(ii);

            bool condensed(false);
            xTft.loadFont(F("NotoSans-Regular30"));
            if ((xTft.textWidth(name) + xTft.textWidth(symbol) + 5) > DISPLAY_WIDTH) {
                xTft.unloadFont();
                xTft.loadFont(F("NotoSans-Condensed30"));
                condensed = true;
            }
            xTft.setTextColor(TFT_WHITE, TFT_BLACK);
            xTft.setCursor(0, y_cursor);
            xTft.print(name);
            xTft.unloadFont();
            if (condensed) {
                xTft.loadFont(F("NotoSans-Condensed25"));
            } else {
                xTft.loadFont(F("NotoSans-Regular25"));
            }
            xTft.setCursor(xTft.getCursorX() + 10, y_cursor + 4);
            xTft.setTextColor(GREY_LEVEL2, TFT_BLACK);
            xTft.print(symbol);
            xTft.unloadFont();
        }
    }
}

void Display::renderCoin()
{
    LOG_FUNC

    gecko_t price;
    gecko_t price2;
    gecko_t change_pct;
    gecko_t market_cap;
    gecko_t volume;

    xGecko.price(m_current_coin_index, price, price2, change_pct, market_cap, volume);

    if (m_last_price_update >= xGecko.lastPriceFetch()) {
        LOG_I_PRINTLN("Prices unchanged - skip");
        m_last_price_update = millis_test();
        return; // omit overwriting price with same values
    }
    LOG_I_PRINTLN("Update price display");

    uint16_t color(change_pct >= 0.0 ? GREEN565 : RED565);

    String msg;
    xTft.setTextColor(color, TFT_BLACK);
    xTft.loadFont(F("NotoSans-Regular50"));
    formatNumber(price, msg, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat());
    addCurrencySmbol(msg, xSettings.currencySymbol(), xSettings.currencySymbolPosition());

    auto priceWidth(xTft.textWidth(msg));
    if (priceWidth > DISPLAY_WIDTH) {
        xTft.unloadFont();
        xTft.loadFont(F("NotoSans-Condensed50"));
        priceWidth = xTft.textWidth(msg);
        if (priceWidth > DISPLAY_WIDTH) {
            xTft.unloadFont();
            xTft.loadFont(F("NotoSans-ExtraCondensed50"));
            priceWidth = xTft.textWidth(msg);
        }
    }

    int32_t y_price(70);
    int32_t y_change(125);
    if (xSettings.mode() == Settings::Mode::MULTIPLE_COINS) {
        y_price += 2;
        y_change += 2;
    }
    xTft.fillRect(0, y_price, DISPLAY_WIDTH - priceWidth, 50, TFT_BLACK);
    xTft.setCursor(DISPLAY_WIDTH - priceWidth, y_price);
    xTft.print(msg);
    xTft.unloadFont();

    String secondCurrency;
    uint16_t secondCurrencyColor(GREY_LEVEL2);
    switch (xSettings.secondCurrencyShow()) {
    case Settings::SecondCurrencyShow::VALUE:
        formatNumber(price2, secondCurrency, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat());
        addCurrencySmbol(secondCurrency, xSettings.currency2Symbol(), xSettings.currencySymbolPosition());
        break;
    case Settings::SecondCurrencyShow::MARKETCAP:
        formatNumber(market_cap, secondCurrency, xSettings.numberFormat(), false, true, CompressNumberFormat::COMPACT_LARGE);
        addCurrencySmbol(secondCurrency, xSettings.currency2Symbol(), xSettings.currencySymbolPosition());
        secondCurrencyColor = TFT_GOLD;
        break;
    case Settings::SecondCurrencyShow::VOLUME:
        formatNumber(volume, secondCurrency, xSettings.numberFormat(), false, true, CompressNumberFormat::COMPACT_LARGE);
        addCurrencySmbol(secondCurrency, xSettings.currency2Symbol(), xSettings.currencySymbolPosition());
        secondCurrencyColor = TFT_SKYBLUE;
        break;
    }
    formatNumber(change_pct, msg, xSettings.numberFormat(), true, false, CompressNumberFormat::NORMAL, 2);
    msg += "%";

    xTft.loadFont(F("NotoSans-Regular25"));
    auto secondCurrencyWidth(xTft.textWidth(secondCurrency));
    auto changePctWidth(xTft.textWidth(msg));

    if ((secondCurrencyWidth + changePctWidth + 15) > DISPLAY_WIDTH) {
        xTft.unloadFont();
        xTft.loadFont(F("NotoSans-Condensed25"));
        secondCurrencyWidth = xTft.textWidth(secondCurrency);
        changePctWidth = xTft.textWidth(msg);
        if ((secondCurrencyWidth + changePctWidth + 15) > DISPLAY_WIDTH) {
            xTft.unloadFont();
            xTft.loadFont(F("NotoSans-ExtraCondensed25"));
            secondCurrencyWidth = xTft.textWidth(secondCurrency);
            changePctWidth = xTft.textWidth(msg);
        }
    }

    xTft.fillRect(0, y_change, DISPLAY_WIDTH - secondCurrencyWidth - changePctWidth - 15, 25, TFT_BLACK);
    xTft.setCursor(DISPLAY_WIDTH - secondCurrencyWidth - changePctWidth - 15, y_change);
    xTft.setTextColor(secondCurrencyColor, TFT_BLACK);
    xTft.print(secondCurrency);

    xTft.fillRect(DISPLAY_WIDTH - changePctWidth - 15, y_change, 15, 25, TFT_BLACK);
    xTft.setCursor(DISPLAY_WIDTH - changePctWidth, y_change);
    xTft.setTextColor(color, TFT_BLACK);
    xTft.print(msg);

    xTft.unloadFont();

    m_last_price_update = millis_test();
}

void Display::renderTwoCoins()
{
    LOG_FUNC

    gecko_t price[2];
    gecko_t price2[2];
    gecko_t change_pct[2];
    gecko_t market_cap[2];
    gecko_t volume[2];
    xGecko.twoPrices(price[0], price2[0], change_pct[0], market_cap[0], volume[0], price[1], price2[1], change_pct[1], market_cap[1], volume[1]);

    if (m_last_price_update >= xGecko.lastPriceFetch()) {
        LOG_I_PRINTLN("Prices unchanged - skip");
        m_last_price_update = millis_test();
        return; // omit overwriting price with same values
    }
    LOG_I_PRINTLN("Update price display");

    for (auto coinIndex : { 0, 1 }) {
        uint16_t color(change_pct[coinIndex] >= 0.0 ? GREEN565 : RED565);

        String msg;
        xTft.setTextColor(color, TFT_BLACK);
        xTft.loadFont(F("NotoSans-Regular50"));
        formatNumber(price[coinIndex], msg, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat());
        addCurrencySmbol(msg, xSettings.currencySymbol(), xSettings.currencySymbolPosition());

        auto priceWidth(xTft.textWidth(msg));
        if (priceWidth > DISPLAY_WIDTH) {
            xTft.unloadFont();
            xTft.loadFont(F("NotoSans-Condensed50"));
            priceWidth = xTft.textWidth(msg);
            if (priceWidth > DISPLAY_WIDTH) {
                xTft.unloadFont();
                xTft.loadFont(F("NotoSans-ExtraCondensed50"));
                priceWidth = xTft.textWidth(msg);
            }
        }
        xTft.fillRect(0, 35 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)), DISPLAY_WIDTH - priceWidth, 50, TFT_BLACK);
        xTft.setCursor(DISPLAY_WIDTH - priceWidth, 35 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)));
        xTft.print(msg);
        xTft.unloadFont();

        String secondCurrency;
        uint16_t secondCurrencyColor(GREY_LEVEL2);
        switch (xSettings.secondCurrencyShow()) {
        case Settings::SecondCurrencyShow::VALUE:
            formatNumber(price2[coinIndex], secondCurrency, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat());
            addCurrencySmbol(secondCurrency, xSettings.currency2Symbol(), xSettings.currencySymbolPosition());
            break;
        case Settings::SecondCurrencyShow::MARKETCAP:
            formatNumber(market_cap[coinIndex], secondCurrency, xSettings.numberFormat(), false, true, CompressNumberFormat::COMPACT_LARGE);
            addCurrencySmbol(secondCurrency, xSettings.currency2Symbol(), xSettings.currencySymbolPosition());
            secondCurrencyColor = TFT_GOLD;
            break;
        case Settings::SecondCurrencyShow::VOLUME:
            formatNumber(volume[coinIndex], secondCurrency, xSettings.numberFormat(), false, true, CompressNumberFormat::COMPACT_LARGE);
            addCurrencySmbol(secondCurrency, xSettings.currency2Symbol(), xSettings.currencySymbolPosition());
            secondCurrencyColor = TFT_SKYBLUE;
            break;
        }
        formatNumber(change_pct[coinIndex], msg, xSettings.numberFormat(), true, false, CompressNumberFormat::NORMAL, 2);
        msg += "%";

        xTft.loadFont(F("NotoSans-Regular30"));
        auto secondCurrencyWidth(xTft.textWidth(secondCurrency));
        auto changePctWidth(xTft.textWidth(msg));

        if ((secondCurrencyWidth + changePctWidth + 15) > DISPLAY_WIDTH) {
            xTft.unloadFont();
            xTft.loadFont(F("NotoSans-Condensed30"));
            secondCurrencyWidth = xTft.textWidth(secondCurrency);
            changePctWidth = xTft.textWidth(msg);

            if ((secondCurrencyWidth + changePctWidth + 15) > DISPLAY_WIDTH) {
                xTft.unloadFont();
                xTft.loadFont(F("NotoSans-ExtraCondensed30"));
                secondCurrencyWidth = xTft.textWidth(secondCurrency);
                changePctWidth = xTft.textWidth(msg);

                if ((secondCurrencyWidth + changePctWidth + 15) > DISPLAY_WIDTH) {
                    xTft.unloadFont();
                    xTft.loadFont(F("NotoSans-ExtraCondensed25"));
                    secondCurrencyWidth = xTft.textWidth(secondCurrency);
                    changePctWidth = xTft.textWidth(msg);
                }
            }
        }

        xTft.fillRect(0, 85 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)), DISPLAY_WIDTH - secondCurrencyWidth - changePctWidth - 15, 25, TFT_BLACK);
        xTft.setCursor(DISPLAY_WIDTH - secondCurrencyWidth - changePctWidth - 15, 85 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)));
        xTft.setTextColor(secondCurrencyColor, TFT_BLACK);
        xTft.print(secondCurrency);

        xTft.fillRect(DISPLAY_WIDTH - changePctWidth - 15, 85 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)), 15, 25, TFT_BLACK);
        xTft.setCursor(DISPLAY_WIDTH - changePctWidth, 85 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)));
        xTft.setTextColor(color, TFT_BLACK);
        xTft.print(msg);

        xTft.unloadFont();
    }
    m_last_price_update = millis_test();
}

bool Display::renderChart(Settings::ChartPeriod chartPeriod)
{
    LOG_FUNC

    const std::vector<gecko_t>* prices(nullptr);
    bool refetched(false);

    String period;
    std::vector<gecko_t>::const_iterator beginIt;
    std::vector<gecko_t>::const_iterator endIt;
    std::vector<gecko_t>::const_iterator it;
    if (chartPeriod == Settings::ChartPeriod::PERIOD_24_H) {
        prices = &xGecko.chart_48h(m_current_coin_index, refetched);
        beginIt = prices->end() - 24;
        period = F("24h");
    } else if (chartPeriod == Settings::ChartPeriod::PERIOD_48_H) {
        prices = &xGecko.chart_48h(m_current_coin_index, refetched);
        beginIt = prices->begin();
        period = F("48h");
    } else if (chartPeriod == Settings::ChartPeriod::PERIOD_30_D) {
        prices = &xGecko.chart_60d(m_current_coin_index, refetched);
        beginIt = prices->end() - 30;
        period = F("30d");
    } else if (chartPeriod == Settings::ChartPeriod::PERIOD_60_D) {
        prices = &xGecko.chart_60d(m_current_coin_index, refetched);
        beginIt = prices->begin();
        period = F("60d");
    } else {
        return false;
    }

    if (prices->empty()) {
        return false;
    }

    if (m_last_chart_period == chartPeriod && !refetched) {
        LOG_I_PRINTLN("Chart unchanged - skip");
        m_last_chart_update = millis_test();
        return true; // omit overwriting the chart with the same values
    }
    LOG_I_PRINTLN("Update chart display");

    int16_t textHeight(14);

    endIt = prices->end();

    gecko_t low = *(std::min_element(beginIt, endIt));
    gecko_t high = *(std::max_element(beginIt, endIt));

    // clear chart area
    xTft.fillRect(0, CHART_Y_START - (textHeight / 2), DISPLAY_WIDTH, CHART_HEIGHT + (textHeight / 2), TFT_BLACK);

    uint8_t xPerValue(DISPLAY_WIDTH / (endIt - beginIt));
    uint8_t xAtLow(0);
    uint8_t xAtHigh(0);

    for (it = beginIt; it != endIt; ++it) {
        if (*it <= low) {
            xAtLow = ((it - beginIt) * xPerValue);
        }
        if (*it >= high) {
            xAtHigh = ((it - beginIt) * xPerValue);
        }
    }

    auto calcY = [&high, &low](gecko_t price) -> uint8_t {
        return (CHART_HEIGHT - ((CHART_HEIGHT / (high - low)) * (price - low))) + CHART_Y_START;
    };

    ///////////////////////////////////////////////////////////
    // draw vertical time lines
    for (uint8_t x = 0; x < DISPLAY_WIDTH; x += 24) {
        xTft.drawLine(x, DISPLAY_HEIGHT, x, CHART_Y_START - (textHeight / 2), CHART_VERTICAL_LINE_COLOR);
    }
    xTft.drawLine(DISPLAY_WIDTH - 1, DISPLAY_HEIGHT, DISPLAY_WIDTH - 1, CHART_Y_START - (textHeight / 2), CHART_VERTICAL_LINE_COLOR);

    ///////////////////////////////////////////////////////////
    // draw period as background
    if (xSettings.chartStyle() != Settings::ChartStyle::HIGH_LOW_FIRST_LAST) {
        String lowNumber;
        formatNumber(low, lowNumber, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat(), std::numeric_limits<uint8_t>::max(), true);
        xTft.loadFont(F("NotoSans-Regular15"));
        int16_t widthLowNumber(xTft.textWidth(lowNumber) + 30);
        xTft.unloadFont();

        xTft.loadFont(F("NotoSans-Regular30"));
        auto widthPeriod(xTft.textWidth(period));
        uint8_t periodX(DISPLAY_WIDTH - widthPeriod);
        if (xAtLow < DISPLAY_WIDTH / 2) {
            if (xAtLow > (DISPLAY_WIDTH - xAtLow - widthLowNumber)) {
                periodX = 0;
            }
        } else {
            if ((xAtLow - widthLowNumber) > (DISPLAY_WIDTH - xAtLow)) {
                periodX = 0;
            }
        }

        xTft.setTextColor(PERIOD_COLOR, TFT_BLACK);
        xTft.setCursor(periodX, (DISPLAY_HEIGHT - 24));
        xTft.print(period);
        xTft.unloadFont();
    }

    xTft.loadFont(F("NotoSans-Regular15"));

    ///////////////////////////////////////////////////////////
    // draw dotted line with first and last price
    uint8_t dotted(0);
    uint8_t yFirst(min<uint8_t>(calcY(*beginIt), DISPLAY_HEIGHT - 1));
    uint8_t yLast(min<uint8_t>(calcY(prices->back()), DISPLAY_HEIGHT - 1));
    for (uint8_t x = 0; x < DISPLAY_WIDTH; ++x) {
        if (dotted >= 0 && dotted < 4) {
            xTft.drawPixel(x, yFirst, CHART_FIRST_COLOR);
            xTft.drawPixel(x, yLast, CHART_LAST_COLOR);
        }
        ++dotted;
        dotted %= 10;
    }

    ///////////////////////////////////////////////////////////
    // draw values as chart
    uint16_t color((*beginIt < prices->back()) ? GREEN565 : RED565);

    size_t x(1);
    for (auto v = beginIt + 1; v != endIt; ++v, ++x) {
        xTft.drawLine((x - 1) * xPerValue, calcY(*(v - 1)), x * xPerValue, calcY(*v), color);
        xTft.drawLine(((x - 1) * xPerValue) + 1, calcY(*(v - 1)), (x * xPerValue) + 1, calcY(*v), color);
        xTft.drawLine((x - 1) * xPerValue, calcY(*(v - 1)) - 1, x * xPerValue, calcY(*v) - 1, color);
    }

    ///////////////////////////////////////////////////////////
    // draw low and high attached to the curve
    if (xSettings.chartStyle() == Settings::ChartStyle::HIGH_LOW) {
        for (auto ii : { 0, 1 }) {
            String number;
            uint8_t xAt;
            uint8_t yMarker;
            if (ii == 0) {
                xAt = xAtHigh;
                yMarker = CHART_Y_START;
                formatNumber(high, number, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat(), std::numeric_limits<uint8_t>::max(), true);
                xTft.setTextColor(CHART_HIGH_COLOR, TFT_BLACK);
            } else {
                xAt = xAtLow;
                yMarker = DISPLAY_HEIGHT - (textHeight / 2);
                formatNumber(low, number, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat(), std::numeric_limits<uint8_t>::max(), true);
                xTft.setTextColor(CHART_LOW_COLOR, TFT_BLACK);
            }
            addCurrencySmbol(number, xSettings.currencySymbol(), xSettings.currencySymbolPosition());

            int16_t widthNumber(xTft.textWidth(number));

            if (xAt < DISPLAY_WIDTH / 2) {
                xTft.fillRect(xAt + 30, yMarker - (textHeight / 2) - DISTANCE_CHART_VALUE,
                    widthNumber + (2 * DISTANCE_CHART_VALUE), textHeight + (2 * DISTANCE_CHART_VALUE), TFT_BLACK);

                xTft.drawLine(xAt - 1, yMarker - 1, xAt + 31, yMarker - 1, TFT_BLACK);
                xTft.drawLine(xAt - 1, yMarker + 1, xAt + 31, yMarker + 1, TFT_BLACK);

                xTft.drawLine(xAt + 1, yMarker, xAt + 6, yMarker - 4, TFT_BLACK);
                xTft.drawLine(xAt - 1, yMarker, xAt + 4, yMarker - 4, TFT_BLACK);

                xTft.drawLine(xAt + 1, yMarker, xAt + 6, yMarker + 4, TFT_BLACK);
                xTft.drawLine(xAt - 1, yMarker, xAt + 4, yMarker + 4, TFT_BLACK);

                xTft.drawLine(xAt, yMarker, xAt + 30, yMarker, GREY_LEVEL2);
                xTft.drawLine(xAt, yMarker, xAt + 5, yMarker - 4, GREY_LEVEL2);
                xTft.drawLine(xAt, yMarker, xAt + 5, yMarker + 4, GREY_LEVEL2);

                xTft.setCursor(xAt + 30 + DISTANCE_CHART_VALUE, yMarker - (textHeight / 2));
                xTft.print(number);
            } else {
                xTft.fillRect(xAt - 30 - widthNumber - (2 * DISTANCE_CHART_VALUE), yMarker - (textHeight / 2) - DISTANCE_CHART_VALUE,
                    widthNumber + (2 * DISTANCE_CHART_VALUE), textHeight + (2 * DISTANCE_CHART_VALUE), TFT_BLACK);

                xTft.drawLine(xAt + 1, yMarker - 1, xAt - 31, yMarker - 1, TFT_BLACK);
                xTft.drawLine(xAt + 1, yMarker + 1, xAt - 31, yMarker + 1, TFT_BLACK);

                xTft.drawLine(xAt + 1, yMarker, xAt - 4, yMarker - 4, TFT_BLACK);
                xTft.drawLine(xAt - 1, yMarker, xAt - 6, yMarker - 4, TFT_BLACK);

                xTft.drawLine(xAt + 1, yMarker, xAt - 4, yMarker + 4, TFT_BLACK);
                xTft.drawLine(xAt - 1, yMarker, xAt - 6, yMarker + 4, TFT_BLACK);

                xTft.drawLine(xAt, yMarker, xAt - 30, yMarker, GREY_LEVEL2);
                xTft.drawLine(xAt, yMarker, xAt - 5, yMarker - 4, GREY_LEVEL2);
                xTft.drawLine(xAt, yMarker, xAt - 5, yMarker + 4, GREY_LEVEL2);

                xTft.setCursor(xAt - 30 - widthNumber - DISTANCE_CHART_VALUE, yMarker - (textHeight / 2));
                xTft.print(number);
            }
        }
    }

    ///////////////////////////////////////////////////////////
    // draw first, last, low and high to box
    if (xSettings.chartStyle() == Settings::ChartStyle::HIGH_LOW_FIRST_LAST) {
        gecko_t first10avg(0.);
        for (auto v = beginIt; v != beginIt + 10; ++v) {
            first10avg += *v;
        }
        first10avg /= 10;

        uint8_t boxY(CHART_Y_START - 3);
        if (first10avg > ((high - low) / 2) + low) {
            boxY = DISPLAY_HEIGHT - (4 * (textHeight + 2)) - 4;
        }

        String numberFirst, numberLast, numberLow, numberHigh;
        formatNumber(*beginIt, numberFirst, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat(), std::numeric_limits<uint8_t>::max(), true);
        formatNumber(prices->back(), numberLast, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat(), std::numeric_limits<uint8_t>::max(), true);
        formatNumber(low, numberLow, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat(), std::numeric_limits<uint8_t>::max(), true);
        formatNumber(high, numberHigh, xSettings.numberFormat(), false, true, xSettings.smallDecimalNumberFormat(), std::numeric_limits<uint8_t>::max(), true);
        addCurrencySmbol(numberHigh, xSettings.currencySymbol(), xSettings.currencySymbolPosition());
        addCurrencySmbol(numberLow, xSettings.currencySymbol(), xSettings.currencySymbolPosition());
        addCurrencySmbol(numberFirst, xSettings.currencySymbol(), xSettings.currencySymbolPosition());
        addCurrencySmbol(numberLast, xSettings.currencySymbol(), xSettings.currencySymbolPosition());

        int16_t widthHigh(xTft.textWidth(numberHigh));
        int16_t widthLow(xTft.textWidth(numberLow));
        int16_t widthFirst(xTft.textWidth(numberFirst));
        int16_t widthLast(xTft.textWidth(numberLast));
        int16_t maxWidth(widthHigh);
        if (widthLow > maxWidth) {
            maxWidth = widthLow;
        }
        if (widthFirst > maxWidth) {
            maxWidth = widthFirst;
        }
        if (widthLast > maxWidth) {
            maxWidth = widthLast;
        }
        xTft.fillRect(20 - 5, boxY - 4, 55 - 20 + maxWidth + 10, (4 * (textHeight + 2)) + 8, CHART_BOX_BG);

        xTft.setTextColor(CHART_HIGH_COLOR, CHART_BOX_BG);
        xTft.setCursor(20, boxY);
        xTft.print("high");
        xTft.setCursor(55 + maxWidth - widthHigh, boxY);
        xTft.print(numberHigh);

        xTft.setTextColor(CHART_LOW_COLOR, CHART_BOX_BG);
        xTft.setCursor(20, boxY + (textHeight + 2));
        xTft.print("low");
        xTft.setCursor(55 + maxWidth - widthLow, boxY + (textHeight + 2));
        xTft.print(numberLow);

        xTft.setTextColor(CHART_FIRST_COLOR, CHART_BOX_BG);
        xTft.setCursor(20, boxY + (2 * (textHeight + 2)));
        xTft.print("first");
        xTft.setCursor(55 + maxWidth - widthFirst, boxY + (2 * (textHeight + 2)));
        xTft.print(numberFirst);

        xTft.setTextColor(CHART_LAST_COLOR, CHART_BOX_BG);
        xTft.setCursor(20, boxY + (3 * (textHeight + 2)));
        xTft.print("last");
        xTft.setCursor(55 + maxWidth - widthLast, boxY + (3 * (textHeight + 2)));
        xTft.print(numberLast);

        xTft.drawRect(20 - 5, boxY - 4, 55 - 20 + maxWidth + 10, (4 * (textHeight + 2)) + 8, CHART_BOX_MARGIN_COLOR);
    }

    ///////////////////////////////////////////////////////////
    // draw chart time period
    if (xSettings.chartStyle() == Settings::ChartStyle::HIGH_LOW_FIRST_LAST) {
        gecko_t last4avg(0.);
        for (auto v = endIt - 4; v != endIt; ++v) {
            last4avg += *v;
        }
        last4avg /= 4;

        uint8_t periodY(CHART_Y_START);
        if (last4avg > ((high - low) / 2) + low) {
            periodY = DISPLAY_HEIGHT - textHeight - DISTANCE_CHART_VALUE;
        }

        auto widthPeriod(xTft.textWidth(period));
        xTft.fillRect(DISPLAY_WIDTH - widthPeriod - (2 * DISTANCE_CHART_VALUE), periodY - DISTANCE_CHART_VALUE,
            widthPeriod + (2 * DISTANCE_CHART_VALUE), textHeight + (2 * DISTANCE_CHART_VALUE), TFT_BLACK);
        xTft.setTextColor(PERIOD_COLOR, TFT_BLACK);
        xTft.setCursor(DISPLAY_WIDTH - widthPeriod, periodY);
        xTft.print(period);
    }
    ///////////////////////////////////////////////////////////
    // done
    xTft.unloadFont();

    m_last_chart_update = millis_test();
    return true;
}

void Display::chartFailed()
{
    if (xGecko.getLastHttpCode() != HTTP_CODE_TOO_MANY_REQUESTS) {
        LOG_I_PRINTLN(F("Chart update failed!"))
        xTft.loadFont(F("NotoSans-Regular20"));
        xTft.setTextColor(TFT_ORANGE, TFT_BLACK);
        xTft.setCursor(10, 175);
        xTft.print(F("Chart update failed!"));

        xTft.setCursor(10, 200);
        xTft.printf("( %d / %u )     ", xGecko.getLastHttpCode(), xGecko.getHttpCount());
        xTft.unloadFont();
    }
}

Settings::ChartPeriod Display::nextChartPeriod() const
{
    Settings::ChartPeriod next(m_last_chart_period);

    if (m_last_chart_period == Settings::ChartPeriod::PERIOD_24_H) {
        if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_48_H) {
            next = Settings::ChartPeriod::PERIOD_48_H;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_30_D) {
            next = Settings::ChartPeriod::PERIOD_30_D;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_60_D) {
            next = Settings::ChartPeriod::PERIOD_60_D;
        }
    } else if (m_last_chart_period == Settings::ChartPeriod::PERIOD_48_H) {
        if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_30_D) {
            next = Settings::ChartPeriod::PERIOD_30_D;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_60_D) {
            next = Settings::ChartPeriod::PERIOD_60_D;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_24_H) {
            next = Settings::ChartPeriod::PERIOD_24_H;
        }
    } else if (m_last_chart_period == Settings::ChartPeriod::PERIOD_30_D) {
        if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_60_D) {
            next = Settings::ChartPeriod::PERIOD_60_D;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_24_H) {
            next = Settings::ChartPeriod::PERIOD_24_H;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_48_H) {
            next = Settings::ChartPeriod::PERIOD_48_H;
        }
    } else if (m_last_chart_period == Settings::ChartPeriod::PERIOD_60_D) {
        if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_24_H) {
            next = Settings::ChartPeriod::PERIOD_24_H;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_48_H) {
            next = Settings::ChartPeriod::PERIOD_48_H;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_30_D) {
            next = Settings::ChartPeriod::PERIOD_30_D;
        }
    } else { // initial
        if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_24_H) {
            next = Settings::ChartPeriod::PERIOD_24_H;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_48_H) {
            next = Settings::ChartPeriod::PERIOD_48_H;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_30_D) {
            next = Settings::ChartPeriod::PERIOD_30_D;
        } else if (xSettings.chartPeriod() & Settings::ChartPeriod::PERIOD_60_D) {
            next = Settings::ChartPeriod::PERIOD_60_D;
        } else {
            next = Settings::ChartPeriod::PERIOD_24_H;
        }
    }

    return next;
}

void Display::nextCoinID()
{
    LOG_FUNC

    LOG_I_PRINTF("last coin: %s [%u] -> ", xSettings.name(m_current_coin_index).c_str(), m_current_coin_index);
    m_current_coin_index = m_current_coin_index + 1;
    if (m_current_coin_index == xSettings.numberCoins()) {
        m_current_coin_index = 0;
    }
    LOG_PRINTF("next coin: %s [%u], number of coins: %u\n", xSettings.name(m_current_coin_index).c_str(), m_current_coin_index, xSettings.numberCoins());
}

void Display::showCoin()
{
    LOG_FUNC

    bool rewrite(false);

    if (m_last_screen != Screen::COIN || xSettings.lastChange() != m_last_seen_settings) {
        LOG_I_PRINTLN("rewrite")
        m_last_chart_period = Settings::ChartPeriod::PERIOD_NONE;
        m_current_coin_index = 0;
        m_last_seen_settings = xSettings.lastChange();
        rewrite = true;
    }

    if (rewrite) {
        renderTitle();
    }

    heartbeat();
    wifiConnect();
    renderCoin();

    uint32_t interval(5 * 1000);
    switch (xSettings.swapInterval()) {
    case Settings::Swap::INTERVAL_1:
        break;
    case Settings::Swap::INTERVAL_2:
        interval = (30 * 1000);
        break;
    case Settings::Swap::INTERVAL_3:
        interval = (60 * 1000);
        break;
    case Settings::Swap::INTERVAL_4:
        interval = (5 * 60 * 1000);
        break;
    }

    if (rewrite || doInterval(m_last_chart_update, interval)) {
        Settings::ChartPeriod next(nextChartPeriod());
        LOG_I_PRINTF("last chartPeriod: %u -> next chartPeriod: %u, setting: %u\n", m_last_chart_period, next, xSettings.chartPeriod());
        if (!renderChart(next)) {
            chartFailed();
            m_last_chart_update = 0;
        }
        m_last_chart_period = next;
    }

    m_last_screen = Screen::COIN;
}

void Display::showTwoCoins()
{
    LOG_FUNC

    bool rewrite(false);

    if (m_last_screen != Screen::COIN || xSettings.lastChange() != m_last_seen_settings) {
        LOG_I_PRINTLN("rewrite")
        m_last_chart_period = Settings::ChartPeriod::PERIOD_NONE;
        m_current_coin_index = 0;
        m_last_seen_settings = xSettings.lastChange();
        rewrite = true;
    }

    if (rewrite) {
        renderTitle();
    }

    heartbeat();
    wifiConnect();
    renderTwoCoins();

    m_last_screen = Screen::COIN;
}

void Display::showMultipleCoins()
{
    LOG_FUNC

    uint32_t interval(10 * 1000);
    switch (xSettings.swapInterval()) {
    case Settings::Swap::INTERVAL_1:
        if (xGecko.increaseIntervalDueToHTTP429()) {
            interval = (15 * 1000);
        }
        break;
    case Settings::Swap::INTERVAL_2:
        interval = (30 * 1000);
        break;
    case Settings::Swap::INTERVAL_3:
        interval = (60 * 1000);
        break;
    case Settings::Swap::INTERVAL_4:
        interval = (5 * 60 * 1000);
        break;
    }

    bool rewrite(false);
    if (m_last_screen != Screen::COIN
        || xSettings.lastChange() != m_last_seen_settings) {
        m_last_chart_period = Settings::ChartPeriod::PERIOD_NONE;
        m_current_coin_index = std::numeric_limits<uint32_t>::max();
        m_last_seen_settings = xSettings.lastChange();
        LOG_I_PRINTLN("rewrite")
        rewrite = true;
    }

    LOG_I_PRINTF("m_last_coin_swap: [%d] \n", (m_last_coin_swap / 1000));

    if (rewrite || m_previous_prefetch_failed || doInterval(m_last_coin_swap, interval)) {
        if (!m_previous_prefetch_failed) {
            nextCoinID();
            m_last_coin_swap = millis_test();
        }
        if (!xGecko.prefetch(m_current_coin_index, static_cast<Settings::ChartPeriod>(xSettings.chartPeriod()))) {
            LOG_I_PRINTLN("Leave, since prefetch was not successful");
            m_previous_prefetch_failed = true;
            return;
        }
        renderTitle();
        m_previous_prefetch_failed = false;
        rewrite = true;
    }

    LOG_I_PRINTF("m_last_price_update: [%d], m_last_chart_update: [%d] \n", (m_last_price_update / 1000), (m_last_chart_update / 1000));

    heartbeat();
    wifiConnect();
    renderCoin();

    if (rewrite || doInterval(m_last_chart_update, interval)) {
        if (!renderChart(static_cast<Settings::ChartPeriod>(xSettings.chartPeriod()))) {
            chartFailed();
            m_last_chart_update = 0;
        }
    }

    m_last_screen = Screen::COIN;
}

void Display::showAPQR()
{
    if (m_last_screen != Screen::AP_QR) {
        xTft.fillScreen(TFT_WHITE);
        xTft.setTextColor(TFT_RED, TFT_WHITE);

        String qrText(F("WIFI:T:WPA;S:"));
        qrText += xHostname;
        qrText += F(";P:");
        qrText += SECRET_AP_PASSWORD;
        qrText += F(";H:;");
        ESP_QRcode tftQR(&xTft);
        tftQR.qrcode(qrText.c_str(), 20, 50, 200, 3);

        xTft.loadFont(F("NotoSans-Regular20"));

        String msg = F("WiFi: ");
        msg += xHostname;
        xTft.setCursor(5, 5);
        xTft.print(msg);
        msg = F("Password: ");
        msg += SECRET_AP_PASSWORD;
        xTft.setCursor(5, 25);
        xTft.print(msg);
        xTft.unloadFont();

        m_last_screen = Screen::AP_QR;
    }
}

void Display::showUpdateQR()
{
    if (m_last_screen != Screen::UPDATE_QR) {
        xTft.fillScreen(TFT_WHITE);
        xTft.setTextColor(TFT_RED, TFT_WHITE);

        String url(F("http://"));
        url += WiFi.localIP().toString().c_str();
        url += F("/update");
        ESP_QRcode tftQR(&xTft);
        tftQR.qrcode(url.c_str(), 20, 50, 200, 3);

        xTft.loadFont(F("NotoSans-Regular20"));
        xTft.setCursor(5, 5);
        xTft.print(F("Open for Update:"));
        xTft.unloadFont();

        xTft.loadFont(F("NotoSans-Regular15"));
        xTft.setCursor(5, 30);
        xTft.print(url);
        xTft.unloadFont();

        m_last_screen = Screen::UPDATE_QR;
    }
}

void Display::showSettingsQR()
{
    if (m_last_screen != Screen::SETTINGS_QR) {
        xTft.fillScreen(TFT_WHITE);
        xTft.setTextColor(TFT_RED, TFT_WHITE);

        String url(F("http://"));
        url += WiFi.localIP().toString().c_str();
        url += F("/");
        ESP_QRcode tftQR(&xTft);
        tftQR.qrcode(url.c_str(), 20, 50, 200, 3);

        xTft.loadFont(F("NotoSans-Regular20"));
        xTft.setCursor(5, 5);
        xTft.print(F("Open Settings:"));

        xTft.setCursor(5, 30);
        xTft.print(url);
        xTft.unloadFont();

        m_last_screen = Screen::SETTINGS_QR;
    }
}

void Display::showCoinThing(String& msg, uint16_t y, bool unloadFont)
{
    xTft.loadFont(F("NotoSans-Regular50"));
    msg = F("CoinThing");
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, y);
    xTft.print(msg);
    if (unloadFont) {
        xTft.unloadFont();
    }
}

void Display::showAPIInfo(String& msg)
{
    xTft.loadFont(F("NotoSans-Regular20"));

#if COIN_THING_SERIAL > 0
    msg = F("Serial Debug");
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 85);
    xTft.print(msg);
#endif

    if (xSettings.isFakeGeckoServer()) {
        msg = xSettings.getGeckoServer();
        xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 150);
        xTft.print(msg);
    }

    msg = VERSION;
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 180);
    xTft.print(msg);

    msg = F("http://");
    msg += WiFi.localIP().toString().c_str();
    msg += "/";
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 210);
    xTft.print(msg);
    xTft.unloadFont();
}

void Display::showAPIOK()
{
    if (m_last_screen != Screen::API_OK) {
        xTft.fillScreen(RGB(0x0, 0x30, 0x90));
        xTft.setTextColor(TFT_WHITE, RGB(0x0, 0x30, 0x90));

        String msg;
        showCoinThing(msg);

        xTft.loadFont(F("NotoSans-Regular30"));
        msg = F("To The Moon!");
        xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 110);
        xTft.print(msg);
        xTft.unloadFont();

        showAPIInfo(msg);

        m_last_screen = Screen::API_OK;
    }
}

void Display::showAPIFailed()
{
    if (m_last_screen != Screen::API_FAILED) {
        xTft.fillScreen(TFT_RED);
        xTft.setTextColor(TFT_BLACK, TFT_RED);

        String msg;
        showCoinThing(msg);

        xTft.loadFont(F("NotoSans-Regular30"));
        msg = F("Gecko API failed!");
        xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 110);
        xTft.print(msg);
        xTft.unloadFont();

        showAPIInfo(msg);

        m_last_screen = Screen::API_FAILED;
    }
}

void Display::showPrepareUpdate(bool failed)
{
    if (failed) {
        xTft.fillScreen(TFT_ORANGE);
        xTft.setTextColor(TFT_BLACK, TFT_ORANGE);
    } else {
        xTft.fillScreen(TFT_DARKGREEN);
        xTft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    }

    String msg;
    showCoinThing(msg, 50);

    xTft.loadFont(F("NotoSans-Regular30"));
    msg = F("Prepare Update");
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 115);
    xTft.print(msg);

    if (failed) {
        msg = F("Failed!");
        xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 160);
        xTft.print(msg);
        xTft.unloadFont();

        xTft.loadFont(F("NotoSans-Regular20"));
        msg = F("Please try again...");
        xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 200);
        xTft.print(msg);
    } else {
        msg = F("Please wait...");
        xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 160);
        xTft.print(msg);
    }
    xTft.unloadFont();
}

void Display::showUpdated()
{
    xTft.fillScreen(RGB(0x0, 0x80, 0x30));
    xTft.setTextColor(TFT_WHITE, RGB(0x0, 0x80, 0x30));

    String msg;
    showCoinThing(msg, 50);

    xTft.loadFont(F("NotoSans-Regular20"));
    msg = F("Updated To Version");
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 115);
    xTft.print(msg);

    msg = VERSION;
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 145);
    xTft.print(msg);
    xTft.unloadFont();
}

void Display::showNotUpdated()
{
    xTft.fillScreen(RGB(0x80, 0x30, 0x0));
    xTft.setTextColor(TFT_WHITE, RGB(0x80, 0x30, 0x0));

    String msg;
    showCoinThing(msg, 50);

    xTft.loadFont(F("NotoSans-Regular30"));
    msg = F("Not Updated");
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 115);
    xTft.print(msg);
    xTft.unloadFont();

    xTft.loadFont(F("NotoSans-Regular20"));
    msg = F("Version");
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 160);
    xTft.print(msg);

    msg = VERSION;
    xTft.setCursor((DISPLAY_WIDTH - xTft.textWidth(msg)) / 2, 190);
    xTft.print(msg);
    xTft.unloadFont();
}

uint16_t read16(File& f)
{
    uint16_t result;
    ((uint8_t*)&result)[0] = f.read(); // LSB
    ((uint8_t*)&result)[1] = f.read(); // MSB
    return result;
}

uint32_t read32(File& f)
{
    uint32_t result;
    ((uint8_t*)&result)[0] = f.read(); // LSB
    ((uint8_t*)&result)[1] = f.read();
    ((uint8_t*)&result)[2] = f.read();
    ((uint8_t*)&result)[3] = f.read(); // MSB
    return result;
}

bool Display::drawBmp(const String& filename, TFT_eSPI& tft, int16_t x, int16_t y)
{
    bool ret(false);
    if (!SPIFFS.exists(filename)) {
        return ret;
    }

    File f = SPIFFS.open(filename, "r");
    uint32_t seekOffset;
    uint16_t w, h, row;

    // https://de.wikipedia.org/wiki/Windows_Bitmap
    if (read16(f) == 0x4D42) { // bitmap signature
        read32(f); // file size 4 bytes
        read32(f); // reserved 4 bytes
        seekOffset = read32(f); // data offset 4 bytes
        read32(f); // info header size 4 bytes
        w = read32(f); // width 4 bytes
        h = read32(f); // height 4 bytes

        if ((read16(f) == 1) // number of planes 2 bytes
            && (read16(f) == 16) // bit count 2 bytes -> R5 G6 B5
            && (read32(f) == 3)) { // compression 4 bytes
            y += h - 1;
            bool oldSwapBytes = tft.getSwapBytes();
            tft.setSwapBytes(true);
            f.seek(seekOffset);

            uint8_t lineBuffer[(w * 2) + ((w * 2) % 4)];
            for (row = 0; row < h; ++row) {
                f.read(lineBuffer, sizeof(lineBuffer));
                tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
            }
            ret = true;
            tft.setSwapBytes(oldSwapBytes);
        }
    }

    f.close();
    return ret;
}
