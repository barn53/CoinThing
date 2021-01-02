#include "display.h"
#include "gecko.h"
#include "handler.h"
#include "http.h"
#include "settings.h"
#include "utils.h"
#include "wifi.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

HttpJson httpJson;
Gecko gecko(httpJson);
Settings settings(gecko);
Display display(settings);
Handler handler(settings);

void setup(void)
{
    Serial.begin(115200);
    display.begin();

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
    Serial.printf("Settings coin:          >%s<\n", settings.coin().c_str());
    Serial.printf("Settings currency:      >%s<\n", settings.currency().c_str());
    Serial.printf("Settings number format: >%u<\n", settings.numberFormat());
    Serial.printf("Settings symbol:        >%s<\n", settings.symbol().c_str());
    Serial.printf("Settings name:          >%s<\n", settings.name().c_str());

    server.onNotFound([]() { // If the client requests any URI
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
