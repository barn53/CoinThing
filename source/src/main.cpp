
#include "common.h"
#include "display.h"
#include "gecko.h"
#include "handler.h"
#include "http_json.h"
#include "pre.h"
#include "settings.h"
#include "utils.h"
#include "wifi_utils.h"
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

ESP8266WebServer server(80);

// strange PlatformIO behavior:
// https://community.platformio.org/t/adafruit-gfx-lib-will-not-build-any-more-pio-5/15776/12
#include <Adafruit_I2CDevice.h>

HttpJson httpJson;
Settings settings;
Gecko gecko(httpJson, settings);
Display display(gecko, settings);
Handler handler(gecko, settings);

String HostName;

void setup(void)
{
    Serial.begin(115200);
    SPIFFS.begin();

    HostName = F("CoinThing-");
    HostName += ESP.getChipId();

#if COIN_THING_SERIAL > 0
    rst_info* ri(ESP.getResetInfoPtr());
    Serial.printf("\n---------------------\n");
    Serial.printf("Reset Info reason:   %u\n", ri->reason);
    Serial.printf("Reset Info exccause: %u", ri->exccause);
    Serial.printf("\n---------------------\n");
#endif

    display.begin();

    WiFiManager wifiManager;
    handleWifiManager(wifiManager, display);

    Serial.printf("\nConnected\n IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf(" Hostname: %s\n", WiFi.hostname().c_str());
    Serial.printf(" MAC: %s\n", WiFi.macAddress().c_str());

    gecko.begin();
    settings.begin(gecko);

#if COIN_THING_SERIAL > 0
    Serial.printf("Settings coin:                >%s<\n", settings.coin());
    Serial.printf("Settings currency:            >%s<\n", settings.currency());
    Serial.printf("Settings number format:       >%u<\n", settings.numberFormat());
    Serial.printf("Settings chart period:        >%u<\n", settings.chartPeriod());
    Serial.printf("Settings chart swap interval: >%u<\n", settings.chartSwapInterval());
    Serial.printf("Settings chart style:         >%u<\n", settings.chartStyle());
    Serial.printf("Settings heart beat:          >%s<\n", (settings.heartbeat() ? "true" : "false"));
    Serial.printf("Settings brightness:          >%u<\n", settings.brightness());
    Serial.printf("Settings name:                >%s<\n", settings.name());
    Serial.printf("Settings symbol:              >%s<\n", settings.symbol());
#endif

    server.onNotFound([]() { // If the client requests any URI
        if (!handler.handleAction()
            && !handler.handleFileRead()) { // send it if it exists
            server.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
        }
    });
    server.begin(); // Actually start the server

    Serial.println(F("\r\nInitialisation done."));
}

void loop()
{
    display.loop();
    server.handleClient();
}
