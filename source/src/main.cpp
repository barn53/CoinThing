#include "config.h"
#include "display.h"
#include "gecko.h"
#include "http.h"
#include "wifi.h"
#include <Arduino.h>

HttpJson httpJson;
Gecko gecko(httpJson);
Config config(gecko);
Display display(config);

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

    if (config.begin()) {
        Serial.println("Configuration valid!");
    } else {
        Serial.println("Configuration invalid!");
    }
    Serial.printf("Configuration coin:     >%s<\n", config.coin().c_str());
    Serial.printf("Configuration currency: >%s<\n", config.currency().c_str());

    Serial.println("\r\nInitialisation done.");

    display.showConfigInvalid();
    delay(2000);
}

void loop()
{
    display.loop();
}
