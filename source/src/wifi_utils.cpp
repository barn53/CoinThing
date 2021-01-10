#include "wifi_utils.h"
#include "pre.h"
#include "secrets.h"
#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#else
#include <HTTPClient.h>
#include <WiFi.h>
#endif

void wifiSleep()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
#ifdef ESP8266
    WiFi.forceSleepBegin();
#else
    WiFi.setSleep(true);
#endif
    delay(1);
}
void wifiWake()
{
#ifdef ESP8266
    WiFi.forceSleepWake();
#else
    WiFi.setSleep(false);
#endif
    delay(1);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
}

void setupWiFi()
{
    wifiWake();

    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
#ifdef ESP8266
    WiFi.hostname(HOST_NAME);
#else
    WiFi.setHostname(HOST_NAME);
#endif

    Serial.print("Connecting ");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.printf("\nConnected\n IP address: %s\n", WiFi.localIP().toString().c_str());
#ifdef ESP8266
    Serial.printf(" Hostname: %s\n", WiFi.hostname().c_str());
    Serial.printf(" MAC: %s\n", WiFi.macAddress().c_str());
#else
    Serial.printf(" Hostname: %s\n", WiFi.getHostname());
    Serial.printf(" MAC: %s\n", WiFi.macAddress().c_str());
#endif
}
