#include "wifi_utils.h"
#include "pre.h"
#include "secrets.h"
#include <Arduino.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

void wifiSleep()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
}
void wifiWake()
{
    WiFi.forceSleepWake();
    delay(1);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
}

void setupWiFi()
{
    wifiWake();

    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
    WiFi.hostname(HOST_NAME);

    Serial.print("Connecting ");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.printf("\nConnected\n IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf(" Hostname: %s\n", WiFi.hostname().c_str());
    Serial.printf(" MAC: %s\n", WiFi.macAddress().c_str());
}
