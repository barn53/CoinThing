#include "selftest.h"
#include "common.h"
#include "display.h"
#include "gecko.h"
#include "http_json.h"
#include <TFT_eSPI.h>

void selftest(Display& display)
{
    Serial.printf("\nS E L F T E S T\n");

    auto& tft(display.getTFT());
    auto& gecko(display.getGecko());
    auto& http(gecko.getHttpJson());

    uint16_t bg(tft.color565(0xf0, 0x00, 0xff));
    tft.fillScreen(bg);

    String txt(F("SELFTEST"));
    tft.loadFont(F("NotoSans-Regular50"));
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_WHITE, bg);
    tft.print(txt);
    tft.unloadFont();

    tft.loadFont(F("NotoSans-Regular30"));
    String pre(F("https://api.coingecko.com/api/v3/simple/price?ids="));
    String post(F("&vs_currencies=usd&include_24hr_change=true"));
    for (const auto* id : { F("bitcoin"), F("ethereum"), F("ripple"), F("cardano"), F("dogecoin") }) {
        String url(pre);
        url += id;
        url += post;

        DynamicJsonDocument doc(384);
        if (http.read(url.c_str(), doc)) {
            auto p(doc[id][F("usd")] | std::numeric_limits<gecko_t>::infinity());
            auto c(doc[id][F("usd_24h_change")] | std::numeric_limits<gecko_t>::infinity());

            String price;
            String change;

            formatNumber(p, price, NumberFormat::THOUSAND_COMMA_DECIMAL_DOT, false, true);
            formatNumber(c, change, NumberFormat::THOUSAND_COMMA_DECIMAL_DOT, true, false, 2);

            tft.setCursor(0, 85);
            txt = "  ";
            txt += id;
            txt += F(":           \n  ");
            txt += price;
            txt += F("$           \n\n  24h: ");
            txt += change;
            txt += F("%           ");
            tft.print(txt);
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
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    int16_t bmpX(0);
    int16_t bmpY(0);
    int16_t bmpCount(0);
    Dir dir = SPIFFS.openDir(F("/"));
    while (dir.next()) {
        auto f(dir.fileName());
        if (f.endsWith(F(".bmp"))) {
            ++bmpCount;
            Display::drawBmp(f, tft, bmpX, bmpY);
            bmpX += 50;
            if (bmpX > 200) {
                bmpX = 0;
                bmpY += 50;
                if (bmpY > 200) {
                    bmpY = 0;
                    tft.setCursor(0, 210);
                    tft.print(bmpCount);
                    delay(500);
                    tft.fillScreen(TFT_BLACK);
                }
            }
        }
    }
    tft.setCursor(0, 210);
    tft.print(bmpCount);
    delay(1500);

    bg = tft.color565(0x00, 0x90, 0x10);
    tft.unloadFont();
    tft.loadFont(F("NotoSans-Regular50"));
    tft.fillScreen(bg);
    tft.setCursor(80, 80);
    tft.setTextColor(TFT_WHITE, bg);
    txt = F("OK!");
    tft.print(txt);

    delay(2500);
    ESP.reset();
}
