#include "selftest.h"
#include "common.h"
#include "display.h"
#include "gecko.h"
#include "http_json.h"
#include "pre.h"
#include <TFT_eSPI.h>

extern TFT_eSPI xTft;
extern Gecko xGecko;
extern HttpJson xHttpJson;

void selftest()
{
    LOG_FUNC
    LOG_I_PRINTLN("")
    LOG_I_PRINTLN("S E L F T E S T")

    uint16_t bg(xTft.color565(0xf0, 0x00, 0xff));
    xTft.fillScreen(bg);

    String msg(F("SELFTEST"));
    xTft.loadFont(F("NotoSans-Regular50"));
    xTft.setCursor((TFT_WIDTH - xTft.textWidth(msg)) / 2, 10);
    xTft.setTextColor(TFT_WHITE, bg);
    xTft.print(msg);
    xTft.unloadFont();

    xTft.loadFont(F("NotoSans-Regular20"));
    msg = F("v: ");
    msg += VERSION;
    size_t textY(65);
    xTft.setCursor((TFT_WIDTH - xTft.textWidth(msg)) / 2, textY);
    xTft.print(msg);
    File file = SPIFFS.open(F("/spiffs.version"), "r");
    if (file.available()) {
        String spiffs(file.readString());
        file.close();
        msg = F("s: ");
        msg += spiffs;
        textY += 22;
        xTft.setCursor((TFT_WIDTH - xTft.textWidth(msg)) / 2, textY);
        xTft.print(msg);
    }
    msg = F("colorset: ");
    msg += Settings::getColorSet();
    textY += 22;
    xTft.setCursor((TFT_WIDTH - xTft.textWidth(msg)) / 2, textY);
    xTft.print(msg);
    xTft.unloadFont();

    xTft.loadFont(F("NotoSans-Regular30"));
    String pre(F("https://api.coingecko.com"));
    pre += F("/api/v3/simple/price?ids=");
    String post(F("&vs_currencies=usd&include_24hr_change=true"));
    for (const auto* id : { F("bitcoin"), F("ethereum"), F("ripple"), F("cardano"), F("dogecoin") }) {
        String url(pre);
        url += id;
        url += post;

        DynamicJsonDocument doc(384);
        if (xHttpJson.read(url.c_str(), doc)) {
            auto p(doc[id][F("usd")] | std::numeric_limits<gecko_t>::infinity());
            auto c(doc[id][F("usd_24h_change")] | std::numeric_limits<gecko_t>::infinity());

            String price;
            String change;

            formatNumber(p, price, NumberFormat::THOUSAND_COMMA_DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
            formatNumber(c, change, NumberFormat::THOUSAND_COMMA_DECIMAL_DOT, true, false, SmallDecimalNumberFormat::NORMAL, 2);

            xTft.setCursor(0, 140);
            msg = "  ";
            msg += id;
            msg += F(":           \n  ");
            msg += price;
            msg += F("$           \n  24h: ");
            msg += change;
            msg += F("%           ");
            xTft.print(msg);
            delay(500);
        }
    }

    for (auto b = 255; b > 0; --b) {
        analogWrite(TFT_BL, b);
        delay(10);
    }
    for (auto b = 0; b <= 255; ++b) {
        analogWrite(TFT_BL, b);
        delay(10);
    }

    xTft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
    for (const auto* font : {
             F("NotoSans-Regular15"),
             F("NotoSans-Regular20"),
             F("NotoSans-Regular25"),
             F("NotoSans-Regular30"),
             F("NotoSans-Regular50"),
             F("NotoSans-Condensed25"),
             F("NotoSans-Condensed30"),
             F("NotoSans-Condensed50"),
             F("NotoSans-ExtraCondensed25"),
             F("NotoSans-ExtraCondensed30"),
             F("NotoSans-ExtraCondensed50") }) {
        xTft.unloadFont();
        xTft.loadFont(font);
        xTft.fillScreen(TFT_BLACK);
        xTft.setCursor(10, 100);
        String f(font);
        f.replace(F("NotoSans-"), (""));
        xTft.print(f);
        delay(300);
    }

    xTft.unloadFont();
    xTft.loadFont(F("NotoSans-Regular30"));
    xTft.fillScreen(TFT_BLACK);
    xTft.setTextColor(TFT_WHITE, TFT_BLACK);
    int16_t bmpX(0);
    int16_t bmpY(0);
    int16_t bmpCount(0);
    Dir dir = SPIFFS.openDir(F("/"));
    while (dir.next()) {
        auto f(dir.fileName());
        if (f.endsWith(F(".bmp"))) {
            ++bmpCount;
            Display::drawBmp(f, xTft, bmpX, bmpY);
            bmpX += 50;
            if (bmpX > 200) {
                bmpX = 0;
                bmpY += 50;
                if (bmpY > 200) {
                    bmpY = 0;
                    xTft.setCursor(0, 210);
                    xTft.print(bmpCount);
                    delay(300);
                    xTft.fillScreen(TFT_BLACK);
                }
            }
        }
    }
    xTft.setCursor(0, 210);
    xTft.print(bmpCount);
    delay(1500);

    bg = xTft.color565(0x00, 0x90, 0x10);
    xTft.unloadFont();
    xTft.loadFont(F("NotoSans-Regular50"));
    xTft.fillScreen(bg);
    xTft.setCursor(80, 80);
    xTft.setTextColor(TFT_WHITE, bg);
    msg = F("OK!");
    xTft.print(msg);

    delay(2500);
    ESP.restart();
}
