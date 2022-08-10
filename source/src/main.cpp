#include "common.h"
#include "display.h"
#include "gecko.h"
#include "handler.h"
#include "http_json.h"
#include "json_store.h"
#include "pre.h"
#include "selftest.h"
#include "settings.h"
#include "utils.h"
#include "wifi_utils.h"
#include <ESP8266WebServer.h>

#ifndef UNIT_TEST

ESP8266WebServer server(80);

// strange PlatformIO behavior:
// https://community.platformio.org/t/adafruit-gfx-lib-will-not-build-any-more-pio-5/15776/12
#include <Adafruit_I2CDevice.h>

TFT_eSPI xTft;

HttpJson xHttpJson;
Settings xSettings;
Gecko xGecko;
Display xDisplay;

String xHostname;
JsonStore xSecrets("/secrets.json");

Handler handler;

void setup(void)
{
    Serial.begin(115200);
    SPIFFS.begin();

    xHostname = F("CoinThing-");
    xHostname += ESP.getChipId();

#if COIN_THING_SERIAL > 0
    rst_info* ri(ESP.getResetInfoPtr());
    LOG_PRINTF("\n---------------------\n");
    LOG_PRINTF("Reset Info reason:   %u\n", ri->reason);
    LOG_PRINTF("Reset Info exccause: %u", ri->exccause);
    LOG_PRINTF("\n---------------------\n");
#endif

    uint8_t counter(xSettings.handlePowerupSequenceForResetBegin());
    xDisplay.begin(counter);

    handleWifi();

    xGecko.begin();
    xSettings.begin();

    if (SPIFFS.exists(SELFTEST_FILE)) {
        SPIFFS.remove(SELFTEST_FILE);
        selftest();
    }

    server.onNotFound([]() { // If the client requests any URI
        if (!handler.handleAction()
            && !handler.handleFileRead()) { // send it if it exists
            server.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
        }
    });
    server.begin(); // Actually start the server

    Serial.println(F("\nInitialisation done."));
    Serial.print(F("IP: "));
    Serial.println(WiFi.localIP().toString());

    if (millis_test() < 5000) {
        // ensure setup duration is at least 5 seconds
        delay(5500 - millis_test());
    }
    xSettings.handlePowerupSequenceForResetEnd(counter);
}

void loop()
{
    xDisplay.loop();
#if COIN_THING_SERIAL > 1
    delay(500);
#endif
    server.handleClient();
}

#endif // UNIT_TEST
