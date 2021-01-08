#include "display.h"
#include "gecko.h"
#include "handler.h"
#include "http.h"
#include "pre.h"
#include "settings.h"
#include "utils.h"
#include "wifi_utils.h"
#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#else
#include <WebServer.h>
WebServer server(80);
#endif

// strange PlatformIO behavior:
// https://community.platformio.org/t/adafruit-gfx-lib-will-not-build-any-more-pio-5/15776/12
#include <Adafruit_I2CDevice.h>

HttpJson httpJson;
Gecko gecko(httpJson);
Settings settings(gecko);
Display display(settings);
Handler handler(settings);

void setup(void)
{
#ifdef ESP8266
    rst_info* ri(ESP.getResetInfoPtr());
    Serial.begin(115200);
    Serial.printf("\n---------------------\n");
    Serial.printf("Reset Info reason:   %u\n", ri->reason);
    Serial.printf("Reset Info exccause: %u", ri->exccause);
    Serial.printf("\n---------------------\n");
#else
#endif

    display.begin();
    display.showAPQR();

    setupWiFi();

    if (gecko.ping()) {
        display.showAPIOK();
        delay(1500);
    } else {
        display.showAPIFailed();
        while (1)
            yield();
    }

    if (settings.begin()) {
        Serial.println("Settings valid!");
    } else {
        Serial.println("Settings invalid!");
    }
    Serial.printf("Settings coin:          >%s<\n", settings.coin());
    Serial.printf("Settings currency:      >%s<\n", settings.currency());
    Serial.printf("Settings number format: >%u<\n", settings.numberFormat());
    Serial.printf("Settings symbol:        >%s<\n", settings.symbol());
    Serial.printf("Settings name:          >%s<\n", settings.name());

    server.onNotFound([]() { // If the client requests any URI
        TRACE;
        if (!handler.handleAction()
            && !handler.handleFileRead()) { // send it if it exists
            server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
        }
    });
    server.begin(); // Actually start the server

    Serial.println("\r\nInitialisation done.");

    display.showSettingsQR();
    delay(1000);
}

void loop()
{
    display.loop();
    server.handleClient();
}
