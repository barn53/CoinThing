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

#define RGB(r, g, b) (m_tft.color565(r, g, b))

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240

#define CHART_Y_START 160
#define CHART_HEIGHT (DISPLAY_HEIGHT - CHART_Y_START)
#define CHART_MIDDLE (CHART_Y_START + (CHART_HEIGHT / 2))
#define DISTANCE_CHART_VALUE 3

#define API_OK_SHOW_TIME (4 * 1000)

extern String HostName;

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

Display::Display(Gecko& gecko, const Settings& settings)
    : m_gecko(gecko)
    , m_settings(settings)
    , m_tft(TFT_eSPI())
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

void Display::begin()
{
    analogWriteRange(std::numeric_limits<uint8_t>::max());
    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, std::numeric_limits<uint8_t>::max());

    uint8_t colorSet = m_settings.getColorSet();
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

    m_tft.begin();
    m_tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
    m_tft.setTextWrap(false);
    m_tft.fillScreen(TFT_BLACK);

    m_tft.setTextColor(TFT_WHITE, TFT_BLACK);
    m_tft.loadFont(F("NotoSans-Regular50"));
    String msg = F("CoinThing");
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 30);
    m_tft.print(msg);
    m_tft.unloadFont();
}

void Display::loop()
{
    LOG_FUNC

    analogWrite(TFT_BL, m_settings.brightness());

    m_gecko.loop();
    if (m_gecko.succeeded()) {
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
            if (m_settings.valid()) {
                if (m_settings.mode() == Settings::Mode::ONE_COIN) {
                    showCoin();
                } else if (m_settings.mode() == Settings::Mode::TWO_COINS) {
                    showTwoCoins();
                } else if (m_settings.mode() == Settings::Mode::MULTIPLE_COINS) {
                    showMultipleCoins();
                }
            } else {
                showSettingsQR();
            }
        }
    } else {
        showAPIFailed();
    }
}

void Display::wifiConnect()
{
    uint8_t yWiFi(0);
    if (m_settings.heartbeat()) {
        yWiFi = 25;
    }

    if (WiFi.isConnected()) {
        if (m_shows_wifi_not_connected) {
            m_tft.fillRect(215, yWiFi, 25, 25, TFT_BLACK);
            m_shows_wifi_not_connected = false;
        }
    } else {
        if (!m_shows_wifi_not_connected) {
            drawBmp(F("/nowifi.bmp"), m_tft, 215, yWiFi);
            m_shows_wifi_not_connected = true;
        }
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
                && (m_heart_beat_count == 0
                    || m_heart_beat_count == 1
                    || m_heart_beat_count == 2))
            || ((millis_test() - m_last_heartbeat) > 1500
                && (m_heart_beat_count == 3)))) {
        if (m_heart_beat_count % 2 == 0) {
            drawBmp(F("/heart2.bmp"), m_tft, 220, 0);
        } else {
            drawBmp(F("/heart.bmp"), m_tft, 220, 0);
        }
        m_last_heartbeat = millis_test();
        ++m_heart_beat_count;
        m_heart_beat_count %= 4;
    }
}

void Display::renderTitle()
{
    LOG_FUNC

    String name;
    String symbol;
    String currency;

    m_tft.fillScreen(TFT_BLACK);

    if (m_settings.mode() != Settings::Mode::TWO_COINS) {
        name = m_settings.name(m_current_coin_index);
        symbol = m_settings.symbol(m_current_coin_index);
        currency = m_settings.currency();

        int16_t x_name(0);
        String icon(F("/"));
        icon += symbol;
        icon += F(".bmp");
        if (drawBmp(icon, m_tft, 0, 0)) {
            x_name = 60;
        }
        uint8_t y_symbol_curr(35);
        if (m_settings.mode() == Settings::Mode::MULTIPLE_COINS) {
            m_tft.loadFont(F("NotoSans-Regular25"));
            y_symbol_curr = 32;
        } else {
            m_tft.loadFont(F("NotoSans-Regular25"));
        }
        m_tft.setTextColor(GREY_LEVEL2, TFT_BLACK);
        m_tft.setCursor(x_name, y_symbol_curr);
        m_tft.print(symbol);
        m_tft.print(" - ");
        m_tft.print(currency);
        m_tft.unloadFont();

        // Write the name at last, so it appears on top of the symbol and currency
        //  when there is a collision
        m_tft.loadFont(F("NotoSans-Regular30"));
        if (m_tft.textWidth(name) > DISPLAY_WIDTH - x_name) {
            m_tft.unloadFont();
            m_tft.loadFont(F("NotoSans-Condensed30"));
        }
        m_tft.setTextColor(TFT_WHITE, TFT_BLACK);
        m_tft.setCursor(x_name, 0);
        m_tft.print(name);
        m_tft.unloadFont();

        if (m_settings.mode() == Settings::Mode::MULTIPLE_COINS) {
            uint32_t count(0);
            for (uint32_t xx = x_name + 4; count < m_settings.numberCoins(); ++count, xx = xx + 13) {
                m_tft.fillCircle(xx, 60, 2, count == m_current_coin_index ? CURRENT_COIN_DOT_COLOR : GREY_LEVEL2);
            }
        }
    } else { // Settings::Mode::TWO_COINS
        for (auto ii : { 0, 1 }) {
            int16_t y_cursor(ii * 127);
            name = m_settings.name(ii);
            symbol = m_settings.symbol(ii);

            bool condensed(false);
            m_tft.loadFont(F("NotoSans-Regular30"));
            if ((m_tft.textWidth(name) + m_tft.textWidth(symbol) + 5) > DISPLAY_WIDTH) {
                m_tft.unloadFont();
                m_tft.loadFont(F("NotoSans-Condensed30"));
                condensed = true;
            }
            m_tft.setTextColor(TFT_WHITE, TFT_BLACK);
            m_tft.setCursor(0, y_cursor);
            m_tft.print(name);
            m_tft.unloadFont();
            if (condensed) {
                m_tft.loadFont(F("NotoSans-Condensed25"));
            } else {
                m_tft.loadFont(F("NotoSans-Regular25"));
            }
            m_tft.setCursor(m_tft.getCursorX() + 10, y_cursor + 4);
            m_tft.setTextColor(GREY_LEVEL2, TFT_BLACK);
            m_tft.print(symbol);
            m_tft.unloadFont();
        }
    }
}

void Display::renderCoin()
{
    LOG_FUNC

    gecko_t price;
    gecko_t price2;
    gecko_t change_pct;
    m_gecko.price(m_current_coin_index, price, price2, change_pct);

    if (m_last_price_update >= m_gecko.lastPriceFetch()) {
        LOG_I_PRINTLN("Prices unchanged - skip");
        return; // omit overwriting price with same values
    }
    LOG_I_PRINTLN("Update price display");

    uint16_t color(change_pct >= 0.0 ? GREEN565 : RED565);

    String msg;
    m_tft.setTextColor(color, TFT_BLACK);
    m_tft.loadFont(F("NotoSans-Regular50"));
    formatNumber(price, msg, m_settings.numberFormat(), false, true);
    msg += m_settings.currencySymbol();

    auto priceWidth(m_tft.textWidth(msg));
    if (priceWidth > DISPLAY_WIDTH) {
        m_tft.unloadFont();
        m_tft.loadFont(F("NotoSans-Condensed50"));
        priceWidth = m_tft.textWidth(msg);
        if (priceWidth > DISPLAY_WIDTH) {
            m_tft.unloadFont();
            m_tft.loadFont(F("NotoSans-ExtraCondensed50"));
            priceWidth = m_tft.textWidth(msg);
        }
    }

    int32_t y_price(70);
    int32_t y_change(125);
    if (m_settings.mode() == Settings::Mode::MULTIPLE_COINS) {
        y_price += 2;
        y_change += 2;
    }
    m_tft.fillRect(0, y_price, DISPLAY_WIDTH - priceWidth, 50, TFT_BLACK);
    m_tft.setCursor(DISPLAY_WIDTH - priceWidth, y_price);
    m_tft.print(msg);
    m_tft.unloadFont();

    String msg2;
    formatNumber(price2, msg2, m_settings.numberFormat(), false, true);
    formatNumber(change_pct, msg, m_settings.numberFormat(), true, false, 2);
    msg2 += m_settings.currency2Symbol();
    msg += "%";
    m_tft.loadFont(F("NotoSans-Regular25"));
    auto usdWidth(m_tft.textWidth(msg2));
    auto changeWidth(m_tft.textWidth(msg));

    if ((usdWidth + changeWidth + 15) > DISPLAY_WIDTH) {
        m_tft.unloadFont();
        m_tft.loadFont(F("NotoSans-Condensed25"));
        usdWidth = m_tft.textWidth(msg2);
        changeWidth = m_tft.textWidth(msg);
        if ((usdWidth + changeWidth + 15) > DISPLAY_WIDTH) {
            m_tft.unloadFont();
            m_tft.loadFont(F("NotoSans-ExtraCondensed25"));
            usdWidth = m_tft.textWidth(msg2);
            changeWidth = m_tft.textWidth(msg);
        }
    }

    m_tft.fillRect(0, y_change, DISPLAY_WIDTH - usdWidth - changeWidth - 15, 25, TFT_BLACK);
    m_tft.setCursor(DISPLAY_WIDTH - usdWidth - changeWidth - 15, y_change);
    m_tft.setTextColor(GREY_LEVEL2, TFT_BLACK);
    m_tft.print(msg2);

    m_tft.fillRect(DISPLAY_WIDTH - changeWidth - 15, y_change, 15, 25, TFT_BLACK);
    m_tft.setCursor(DISPLAY_WIDTH - changeWidth, y_change);
    m_tft.setTextColor(color, TFT_BLACK);
    m_tft.print(msg);

    m_tft.unloadFont();

    m_last_price_update = millis_test();
}

void Display::renderTwoCoins()
{
    LOG_FUNC

    gecko_t price[2];
    gecko_t price2[2];
    gecko_t change_pct[2];
    m_gecko.twoPrices(price[0], price2[0], change_pct[0], price[1], price2[1], change_pct[1]);

    if (m_last_price_update >= m_gecko.lastPriceFetch()) {
        LOG_I_PRINTLN("Prices unchanged - skip");
        m_last_price_update = millis_test();
        return; // omit overwriting price with same values
    }
    LOG_I_PRINTLN("Update price display");

    for (auto coinIndex : { 0, 1 }) {
        uint16_t color(change_pct[coinIndex] >= 0.0 ? GREEN565 : RED565);

        String msg;
        m_tft.setTextColor(color, TFT_BLACK);
        m_tft.loadFont(F("NotoSans-Regular50"));
        formatNumber(price[coinIndex], msg, m_settings.numberFormat(), false, true);
        msg += m_settings.currencySymbol();

        auto priceWidth(m_tft.textWidth(msg));
        if (priceWidth > DISPLAY_WIDTH) {
            m_tft.unloadFont();
            m_tft.loadFont(F("NotoSans-Condensed50"));
            priceWidth = m_tft.textWidth(msg);
            if (priceWidth > DISPLAY_WIDTH) {
                m_tft.unloadFont();
                m_tft.loadFont(F("NotoSans-ExtraCondensed50"));
                priceWidth = m_tft.textWidth(msg);
            }
        }
        m_tft.fillRect(0, 35 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)), DISPLAY_WIDTH - priceWidth, 50, TFT_BLACK);
        m_tft.setCursor(DISPLAY_WIDTH - priceWidth, 35 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)));
        m_tft.print(msg);
        m_tft.unloadFont();

        String msg2;
        formatNumber(price2[coinIndex], msg2, m_settings.numberFormat(), false, true);
        formatNumber(change_pct[coinIndex], msg, m_settings.numberFormat(), true, false, 2);
        msg2 += m_settings.currency2Symbol();
        msg += "%";
        m_tft.loadFont(F("NotoSans-Regular30"));
        auto usdWidth(m_tft.textWidth(msg2));
        auto changeWidth(m_tft.textWidth(msg));

        if ((usdWidth + changeWidth + 15) > DISPLAY_WIDTH) {
            m_tft.unloadFont();
            m_tft.loadFont(F("NotoSans-Condensed30"));
            usdWidth = m_tft.textWidth(msg2);
            changeWidth = m_tft.textWidth(msg);

            if ((usdWidth + changeWidth + 15) > DISPLAY_WIDTH) {
                m_tft.unloadFont();
                m_tft.loadFont(F("NotoSans-ExtraCondensed30"));
                usdWidth = m_tft.textWidth(msg2);
                changeWidth = m_tft.textWidth(msg);

                if ((usdWidth + changeWidth + 15) > DISPLAY_WIDTH) {
                    m_tft.unloadFont();
                    m_tft.loadFont(F("NotoSans-ExtraCondensed25"));
                    usdWidth = m_tft.textWidth(msg2);
                    changeWidth = m_tft.textWidth(msg);
                }
            }
        }

        m_tft.fillRect(0, 85 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)), DISPLAY_WIDTH - usdWidth - changeWidth - 15, 25, TFT_BLACK);
        m_tft.setCursor(DISPLAY_WIDTH - usdWidth - changeWidth - 15, 85 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)));
        m_tft.setTextColor(GREY_LEVEL2, TFT_BLACK);
        m_tft.print(msg2);

        m_tft.fillRect(DISPLAY_WIDTH - changeWidth - 15, 85 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)), 15, 25, TFT_BLACK);
        m_tft.setCursor(DISPLAY_WIDTH - changeWidth, 85 + (coinIndex * ((DISPLAY_HEIGHT / 2) + 7)));
        m_tft.setTextColor(color, TFT_BLACK);
        m_tft.print(msg);

        m_tft.unloadFont();
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
        prices = &m_gecko.chart_48h(m_current_coin_index, refetched);
        beginIt = prices->end() - 24;
        period = F("24h");
    } else if (chartPeriod == Settings::ChartPeriod::PERIOD_48_H) {
        prices = &m_gecko.chart_48h(m_current_coin_index, refetched);
        beginIt = prices->begin();
        period = F("48h");
    } else if (chartPeriod == Settings::ChartPeriod::PERIOD_30_D) {
        prices = &m_gecko.chart_60d(m_current_coin_index, refetched);
        beginIt = prices->end() - 30;
        period = F("30d");
    } else if (chartPeriod == Settings::ChartPeriod::PERIOD_60_D) {
        prices = &m_gecko.chart_60d(m_current_coin_index, refetched);
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
    m_tft.fillRect(0, CHART_Y_START - (textHeight / 2), DISPLAY_WIDTH, CHART_HEIGHT + (textHeight / 2), TFT_BLACK);

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
        m_tft.drawLine(x, DISPLAY_HEIGHT, x, CHART_Y_START - (textHeight / 2), CHART_VERTICAL_LINE_COLOR);
    }
    m_tft.drawLine(DISPLAY_WIDTH - 1, DISPLAY_HEIGHT, DISPLAY_WIDTH - 1, CHART_Y_START - (textHeight / 2), CHART_VERTICAL_LINE_COLOR);

    ///////////////////////////////////////////////////////////
    // draw period as background
    if (m_settings.chartStyle() != Settings::ChartStyle::HIGH_LOW_FIRST_LAST) {
        String lowNumber;
        formatNumber(low, lowNumber, m_settings.numberFormat(), false, true);
        m_tft.loadFont(F("NotoSans-Regular15"));
        int16_t widthLowNumber(m_tft.textWidth(lowNumber) + 30);
        m_tft.unloadFont();

        m_tft.loadFont(F("NotoSans-Regular30"));
        auto widthPeriod(m_tft.textWidth(period));
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

        m_tft.setTextColor(PERIOD_COLOR, TFT_BLACK);
        m_tft.setCursor(periodX, (DISPLAY_HEIGHT - 24));
        m_tft.print(period);
        m_tft.unloadFont();
    }

    m_tft.loadFont(F("NotoSans-Regular15"));

    ///////////////////////////////////////////////////////////
    // draw dotted line with first and last price
    uint8_t dotted(0);
    uint8_t yFirst(min<uint8_t>(calcY(*beginIt), DISPLAY_HEIGHT - 1));
    uint8_t yLast(min<uint8_t>(calcY(prices->back()), DISPLAY_HEIGHT - 1));
    for (uint8_t x = 0; x < DISPLAY_WIDTH; ++x) {
        if (dotted >= 0 && dotted < 4) {
            m_tft.drawPixel(x, yFirst, CHART_FIRST_COLOR);
            m_tft.drawPixel(x, yLast, CHART_LAST_COLOR);
        }
        ++dotted;
        dotted %= 10;
    }

    ///////////////////////////////////////////////////////////
    // draw values as chart
    uint16_t color((*beginIt < prices->back()) ? GREEN565 : RED565);

    size_t x(1);
    for (auto v = beginIt + 1; v != endIt; ++v, ++x) {
        m_tft.drawLine((x - 1) * xPerValue, calcY(*(v - 1)), x * xPerValue, calcY(*v), color);
        m_tft.drawLine(((x - 1) * xPerValue) + 1, calcY(*(v - 1)), (x * xPerValue) + 1, calcY(*v), color);
        m_tft.drawLine((x - 1) * xPerValue, calcY(*(v - 1)) - 1, x * xPerValue, calcY(*v) - 1, color);
    }

    ///////////////////////////////////////////////////////////
    // draw low and high attached to the curve
    if (m_settings.chartStyle() == Settings::ChartStyle::HIGH_LOW) {
        for (auto ii : { 0, 1 }) {
            String number;
            uint8_t xAt;
            uint8_t yMarker;
            if (ii == 0) {
                xAt = xAtHigh;
                yMarker = CHART_Y_START;
                formatNumber(high, number, m_settings.numberFormat(), false, true);
                m_tft.setTextColor(CHART_HIGH_COLOR, TFT_BLACK);
            } else {
                xAt = xAtLow;
                yMarker = DISPLAY_HEIGHT - (textHeight / 2);
                formatNumber(low, number, m_settings.numberFormat(), false, true);
                m_tft.setTextColor(CHART_LOW_COLOR, TFT_BLACK);
            }
            number += m_settings.currencySymbol();
            int16_t widthNumber(m_tft.textWidth(number));

            if (xAt < DISPLAY_WIDTH / 2) {
                m_tft.fillRect(xAt + 30, yMarker - (textHeight / 2) - DISTANCE_CHART_VALUE,
                    widthNumber + (2 * DISTANCE_CHART_VALUE), textHeight + (2 * DISTANCE_CHART_VALUE), TFT_BLACK);

                m_tft.drawLine(xAt - 1, yMarker - 1, xAt + 31, yMarker - 1, TFT_BLACK);
                m_tft.drawLine(xAt - 1, yMarker + 1, xAt + 31, yMarker + 1, TFT_BLACK);

                m_tft.drawLine(xAt + 1, yMarker, xAt + 6, yMarker - 4, TFT_BLACK);
                m_tft.drawLine(xAt - 1, yMarker, xAt + 4, yMarker - 4, TFT_BLACK);

                m_tft.drawLine(xAt + 1, yMarker, xAt + 6, yMarker + 4, TFT_BLACK);
                m_tft.drawLine(xAt - 1, yMarker, xAt + 4, yMarker + 4, TFT_BLACK);

                m_tft.drawLine(xAt, yMarker, xAt + 30, yMarker, GREY_LEVEL2);
                m_tft.drawLine(xAt, yMarker, xAt + 5, yMarker - 4, GREY_LEVEL2);
                m_tft.drawLine(xAt, yMarker, xAt + 5, yMarker + 4, GREY_LEVEL2);

                m_tft.setCursor(xAt + 30 + DISTANCE_CHART_VALUE, yMarker - (textHeight / 2));
                m_tft.print(number);
            } else {
                m_tft.fillRect(xAt - 30 - widthNumber - (2 * DISTANCE_CHART_VALUE), yMarker - (textHeight / 2) - DISTANCE_CHART_VALUE,
                    widthNumber + (2 * DISTANCE_CHART_VALUE), textHeight + (2 * DISTANCE_CHART_VALUE), TFT_BLACK);

                m_tft.drawLine(xAt + 1, yMarker - 1, xAt - 31, yMarker - 1, TFT_BLACK);
                m_tft.drawLine(xAt + 1, yMarker + 1, xAt - 31, yMarker + 1, TFT_BLACK);

                m_tft.drawLine(xAt + 1, yMarker, xAt - 4, yMarker - 4, TFT_BLACK);
                m_tft.drawLine(xAt - 1, yMarker, xAt - 6, yMarker - 4, TFT_BLACK);

                m_tft.drawLine(xAt + 1, yMarker, xAt - 4, yMarker + 4, TFT_BLACK);
                m_tft.drawLine(xAt - 1, yMarker, xAt - 6, yMarker + 4, TFT_BLACK);

                m_tft.drawLine(xAt, yMarker, xAt - 30, yMarker, GREY_LEVEL2);
                m_tft.drawLine(xAt, yMarker, xAt - 5, yMarker - 4, GREY_LEVEL2);
                m_tft.drawLine(xAt, yMarker, xAt - 5, yMarker + 4, GREY_LEVEL2);

                m_tft.setCursor(xAt - 30 - widthNumber - DISTANCE_CHART_VALUE, yMarker - (textHeight / 2));
                m_tft.print(number);
            }
        }
    }

    ///////////////////////////////////////////////////////////
    // draw first, last, low and high to box
    if (m_settings.chartStyle() == Settings::ChartStyle::HIGH_LOW_FIRST_LAST) {
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
        formatNumber(*beginIt, numberFirst, m_settings.numberFormat(), false, true);
        formatNumber(prices->back(), numberLast, m_settings.numberFormat(), false, true);
        formatNumber(low, numberLow, m_settings.numberFormat(), false, true);
        formatNumber(high, numberHigh, m_settings.numberFormat(), false, true);
        numberHigh += m_settings.currencySymbol();
        numberLow += m_settings.currencySymbol();
        numberFirst += m_settings.currencySymbol();
        numberLast += m_settings.currencySymbol();
        int16_t widthHigh(m_tft.textWidth(numberHigh));
        int16_t widthLow(m_tft.textWidth(numberLow));
        int16_t widthFirst(m_tft.textWidth(numberFirst));
        int16_t widthLast(m_tft.textWidth(numberLast));
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
        m_tft.fillRect(20 - 5, boxY - 4, 55 - 20 + maxWidth + 10, (4 * (textHeight + 2)) + 8, CHART_BOX_BG);

        m_tft.setTextColor(CHART_HIGH_COLOR, CHART_BOX_BG);
        m_tft.setCursor(20, boxY);
        m_tft.print("high");
        m_tft.setCursor(55 + maxWidth - widthHigh, boxY);
        m_tft.print(numberHigh);

        m_tft.setTextColor(CHART_LOW_COLOR, CHART_BOX_BG);
        m_tft.setCursor(20, boxY + (textHeight + 2));
        m_tft.print("low");
        m_tft.setCursor(55 + maxWidth - widthLow, boxY + (textHeight + 2));
        m_tft.print(numberLow);

        m_tft.setTextColor(CHART_FIRST_COLOR, CHART_BOX_BG);
        m_tft.setCursor(20, boxY + (2 * (textHeight + 2)));
        m_tft.print("first");
        m_tft.setCursor(55 + maxWidth - widthFirst, boxY + (2 * (textHeight + 2)));
        m_tft.print(numberFirst);

        m_tft.setTextColor(CHART_LAST_COLOR, CHART_BOX_BG);
        m_tft.setCursor(20, boxY + (3 * (textHeight + 2)));
        m_tft.print("last");
        m_tft.setCursor(55 + maxWidth - widthLast, boxY + (3 * (textHeight + 2)));
        m_tft.print(numberLast);

        m_tft.drawRect(20 - 5, boxY - 4, 55 - 20 + maxWidth + 10, (4 * (textHeight + 2)) + 8, CHART_BOX_MARGIN_COLOR);
    }

    ///////////////////////////////////////////////////////////
    // draw chart time period
    if (m_settings.chartStyle() == Settings::ChartStyle::HIGH_LOW_FIRST_LAST) {
        gecko_t last4avg(0.);
        for (auto v = endIt - 4; v != endIt; ++v) {
            last4avg += *v;
        }
        last4avg /= 4;

        uint8_t periodY(CHART_Y_START);
        if (last4avg > ((high - low) / 2) + low) {
            periodY = DISPLAY_HEIGHT - textHeight - DISTANCE_CHART_VALUE;
        }

        auto widthPeriod(m_tft.textWidth(period));
        m_tft.fillRect(DISPLAY_WIDTH - widthPeriod - (2 * DISTANCE_CHART_VALUE), periodY - DISTANCE_CHART_VALUE,
            widthPeriod + (2 * DISTANCE_CHART_VALUE), textHeight + (2 * DISTANCE_CHART_VALUE), TFT_BLACK);
        m_tft.setTextColor(PERIOD_COLOR, TFT_BLACK);
        m_tft.setCursor(DISPLAY_WIDTH - widthPeriod, periodY);
        m_tft.print(period);
    }
    ///////////////////////////////////////////////////////////
    // done
    m_tft.unloadFont();

    m_last_chart_update = millis_test();
    return true;
}

void Display::chartFailed()
{
    LOG_I_PRINTLN(F("Chart update failed!"))

    m_tft.loadFont(F("NotoSans-Regular20"));
    m_tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    m_tft.setCursor(10, 185);
    m_tft.print(F("Chart update failed!"));
    m_tft.unloadFont();
}

Settings::ChartPeriod Display::nextChartPeriod() const
{
    Settings::ChartPeriod next(m_last_chart_period);

    if (m_last_chart_period == Settings::ChartPeriod::PERIOD_24_H) {
        if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_48_H) {
            next = Settings::ChartPeriod::PERIOD_48_H;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_30_D) {
            next = Settings::ChartPeriod::PERIOD_30_D;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_60_D) {
            next = Settings::ChartPeriod::PERIOD_60_D;
        }
    } else if (m_last_chart_period == Settings::ChartPeriod::PERIOD_48_H) {
        if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_30_D) {
            next = Settings::ChartPeriod::PERIOD_30_D;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_60_D) {
            next = Settings::ChartPeriod::PERIOD_60_D;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_24_H) {
            next = Settings::ChartPeriod::PERIOD_24_H;
        }
    } else if (m_last_chart_period == Settings::ChartPeriod::PERIOD_30_D) {
        if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_60_D) {
            next = Settings::ChartPeriod::PERIOD_60_D;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_24_H) {
            next = Settings::ChartPeriod::PERIOD_24_H;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_48_H) {
            next = Settings::ChartPeriod::PERIOD_48_H;
        }
    } else if (m_last_chart_period == Settings::ChartPeriod::PERIOD_60_D) {
        if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_24_H) {
            next = Settings::ChartPeriod::PERIOD_24_H;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_48_H) {
            next = Settings::ChartPeriod::PERIOD_48_H;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_30_D) {
            next = Settings::ChartPeriod::PERIOD_30_D;
        }
    } else { // initial
        if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_24_H) {
            next = Settings::ChartPeriod::PERIOD_24_H;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_48_H) {
            next = Settings::ChartPeriod::PERIOD_48_H;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_30_D) {
            next = Settings::ChartPeriod::PERIOD_30_D;
        } else if (m_settings.chartPeriod() & Settings::ChartPeriod::PERIOD_60_D) {
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

    LOG_I_PRINTF("last coin index: %u -> ", m_current_coin_index);
    m_current_coin_index = m_current_coin_index + 1;
    if (m_current_coin_index == m_settings.numberCoins()) {
        m_current_coin_index = 0;
    }
    LOG_PRINTF("next coin index: %u, number of coins: %u\n", m_current_coin_index, m_settings.numberCoins());
}

void Display::showCoin()
{
    LOG_FUNC

    bool rewrite(false);

    if (m_last_screen != Screen::COIN || m_settings.lastChange() != m_last_seen_settings) {
        LOG_I_PRINTLN("rewrite")
        m_last_chart_period = Settings::ChartPeriod::PERIOD_NONE;
        m_current_coin_index = 0;
        m_last_seen_settings = m_settings.lastChange();
        m_shows_wifi_not_connected = false;
        rewrite = true;
    }

    if (rewrite) {
        renderTitle();
    }

    heartbeat();
    wifiConnect();
    renderCoin();

    uint32_t interval(5 * 1000);
    switch (m_settings.swapInterval()) {
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
        LOG_I_PRINTF("last chartPeriod: %u -> next chartPeriod: %u, setting: %u\n", m_last_chart_period, next, m_settings.chartPeriod());
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

    if (m_last_screen != Screen::COIN || m_settings.lastChange() != m_last_seen_settings) {
        LOG_I_PRINTLN("rewrite")
        m_last_chart_period = Settings::ChartPeriod::PERIOD_NONE;
        m_current_coin_index = 0;
        m_last_seen_settings = m_settings.lastChange();
        m_shows_wifi_not_connected = false;
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
    switch (m_settings.swapInterval()) {
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

    bool rewrite(false);
    if (m_last_screen != Screen::COIN
        || m_settings.lastChange() != m_last_seen_settings) {
        m_last_chart_period = Settings::ChartPeriod::PERIOD_NONE;
        m_current_coin_index = std::numeric_limits<uint32_t>::max();
        m_last_seen_settings = m_settings.lastChange();
        m_shows_wifi_not_connected = false;
        LOG_I_PRINTLN("rewrite")
        rewrite = true;
    }

    LOG_I_PRINTF("m_last_coin_swap: [%d] \n", (m_last_coin_swap / 1000));

    if (rewrite || doInterval(m_last_coin_swap, interval)) {
        nextCoinID();
        m_gecko.prefetch(m_current_coin_index, static_cast<Settings::ChartPeriod>(m_settings.chartPeriod()));
        renderTitle();
        m_last_coin_swap = millis_test();
        rewrite = true;
    }

    LOG_I_PRINTF("m_last_price_update: [%d], m_last_chart_update: [%d] \n", (m_last_price_update / 1000), (m_last_chart_update / 1000));

    heartbeat();
    wifiConnect();
    renderCoin();

    if (rewrite || doInterval(m_last_chart_update, interval)) {
        if (!renderChart(static_cast<Settings::ChartPeriod>(m_settings.chartPeriod()))) {
            chartFailed();
            m_last_chart_update = 0;
        }
    }

    m_last_screen = Screen::COIN;
}

void Display::showAPQR()
{
    if (m_last_screen != Screen::AP_QR) {
        m_tft.fillScreen(TFT_WHITE);
        m_tft.setTextColor(TFT_RED, TFT_WHITE);

        String qrText(F("WIFI:T:WPA;S:"));
        qrText += HostName;
        qrText += F(";P:");
        qrText += SECRET_AP_PASSWORD;
        qrText += F(";H:;");
        ESP_QRcode tftQR(&m_tft);
        tftQR.qrcode(qrText.c_str(), 20, 50, 200, 3);

        m_tft.loadFont(F("NotoSans-Regular20"));

        String msg = F("WiFi: ");
        msg += HostName;
        m_tft.setCursor(5, 5);
        m_tft.print(msg);
        msg = F("Password: ");
        msg += SECRET_AP_PASSWORD;
        m_tft.setCursor(5, 25);
        m_tft.print(msg);
        m_tft.unloadFont();

        m_last_screen = Screen::AP_QR;
    }
}

void Display::showUpdateQR()
{
    if (m_last_screen != Screen::UPDATE_QR) {
        m_tft.fillScreen(TFT_WHITE);
        m_tft.setTextColor(TFT_RED, TFT_WHITE);

        String url(F("http://"));
        url += WiFi.localIP().toString().c_str();
        url += F("/update");
        ESP_QRcode tftQR(&m_tft);
        tftQR.qrcode(url.c_str(), 20, 50, 200, 3);

        m_tft.loadFont(F("NotoSans-Regular20"));
        m_tft.setCursor(5, 5);
        m_tft.print(F("Open for Update:"));
        m_tft.unloadFont();

        m_tft.loadFont(F("NotoSans-Regular15"));
        m_tft.setCursor(5, 30);
        m_tft.print(url);
        m_tft.unloadFont();

        m_last_screen = Screen::UPDATE_QR;
    }
}

void Display::showSettingsQR()
{
    if (m_last_screen != Screen::SETTINGS_QR) {
        m_tft.fillScreen(TFT_WHITE);
        m_tft.setTextColor(TFT_RED, TFT_WHITE);

        String url(F("http://"));
        url += WiFi.localIP().toString().c_str();
        url += "/";
        ESP_QRcode tftQR(&m_tft);
        tftQR.qrcode(url.c_str(), 20, 50, 200, 3);

        m_tft.loadFont(F("NotoSans-Regular20"));
        m_tft.setCursor(5, 5);
        m_tft.print(F("Open Settings:"));

        m_tft.setCursor(5, 30);
        m_tft.print(url);
        m_tft.unloadFont();

        m_last_screen = Screen::SETTINGS_QR;
    }
}

void Display::showAPIOK()
{
    if (m_last_screen != Screen::API_OK) {
        m_tft.fillScreen(RGB(0x0, 0x30, 0x90));
        m_tft.setTextColor(TFT_WHITE, RGB(0x0, 0x30, 0x90));

        m_tft.loadFont(F("NotoSans-Regular50"));
        String msg = F("CoinThing");
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 30);
        m_tft.print(msg);
        m_tft.unloadFont();

        m_tft.loadFont(F("NotoSans-Regular30"));
        msg = F("To The Moon!");
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 110);
        m_tft.print(msg);
        m_tft.unloadFont();

        m_tft.loadFont(F("NotoSans-Regular20"));
        msg = VERSION;
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 180);
        m_tft.print(msg);

        msg = F("http://");
        msg += WiFi.localIP().toString().c_str();
        msg += "/";
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 210);
        m_tft.print(msg);
        m_tft.unloadFont();

        m_last_screen = Screen::API_OK;
    }
}

void Display::showAPIFailed()
{
    if (m_last_screen != Screen::API_FAILED) {
        m_tft.fillScreen(TFT_RED);
        m_tft.setTextColor(TFT_BLACK, TFT_RED);

        m_tft.loadFont(F("NotoSans-Regular30"));
        String msg = F("Gecko API failed!");
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 95);
        m_tft.print(msg);
        m_tft.unloadFont();

        m_last_screen = Screen::API_FAILED;
    }
}

void Display::showPrepareUpdate(bool failed)
{
    if (failed) {
        m_tft.fillScreen(TFT_ORANGE);
        m_tft.setTextColor(TFT_BLACK, TFT_ORANGE);
    } else {
        m_tft.fillScreen(TFT_DARKGREEN);
        m_tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    }

    m_tft.loadFont(F("NotoSans-Regular50"));
    String msg = F("CoinThing");
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 50);
    m_tft.print(msg);
    m_tft.unloadFont();

    m_tft.loadFont(F("NotoSans-Regular30"));
    msg = F("Prepare Update");
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 115);
    m_tft.print(msg);

    if (failed) {
        msg = F("Failed!");
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 160);
        m_tft.print(msg);
        m_tft.unloadFont();

        m_tft.loadFont(F("NotoSans-Regular20"));
        msg = F("Please try again...");
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 200);
        m_tft.print(msg);
    } else {
        msg = F("Please wait...");
        m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 160);
        m_tft.print(msg);
    }
    m_tft.unloadFont();
}

void Display::showUpdated()
{
    m_tft.fillScreen(RGB(0x0, 0x80, 0x30));
    m_tft.setTextColor(TFT_WHITE, RGB(0x0, 0x80, 0x30));

    m_tft.loadFont(F("NotoSans-Regular50"));
    String msg = F("CoinThing");
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 50);
    m_tft.print(msg);
    m_tft.unloadFont();

    m_tft.loadFont(F("NotoSans-Regular20"));
    msg = F("Updated To Version");
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 115);
    m_tft.print(msg);

    msg = VERSION;
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 145);
    m_tft.print(msg);
    m_tft.unloadFont();
}

void Display::showNotUpdated()
{
    m_tft.fillScreen(RGB(0x80, 0x30, 0x0));
    m_tft.setTextColor(TFT_WHITE, RGB(0x80, 0x30, 0x0));

    m_tft.loadFont(F("NotoSans-Regular50"));
    String msg = F("CoinThing");
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 50);
    m_tft.print(msg);
    m_tft.unloadFont();

    m_tft.loadFont(F("NotoSans-Regular30"));
    msg = F("Not Updated");
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 115);
    m_tft.print(msg);
    m_tft.unloadFont();

    m_tft.loadFont(F("NotoSans-Regular20"));
    msg = F("Version");
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 160);
    m_tft.print(msg);

    msg = VERSION;
    m_tft.setCursor((DISPLAY_WIDTH - m_tft.textWidth(msg)) / 2, 190);
    m_tft.print(msg);
    m_tft.unloadFont();
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
