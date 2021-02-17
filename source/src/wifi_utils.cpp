#include "wifi_utils.h"
#include "common.h"
#include "display.h"
#include "pre.h"
#include "secrets.h"
#include "utils.h"
#include <Arduino.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

extern String HostName;

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
    WiFi.hostname(HostName);

    Serial.print(F("Connecting "));
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.printf("\nConnected\n IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf(" Hostname: %s\n", WiFi.hostname().c_str());
    Serial.printf(" MAC: %s\n", WiFi.macAddress().c_str());
}

void handleWifiManager(WiFiManager& wifiManager, Display& display)
{
    bool forUpdate(false);

    WiFi.hostname(HostName);
    if (SPIFFS.exists(FOR_UPDATE_FILE)) {
        SPIFFS.remove(FOR_UPDATE_FILE);
        SERIAL_PRINTLN(F("CoinThing in update mode"));
        forUpdate = true;
        wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
            SERIAL_PRINTLN("AP Callback For Update");
            display.showUpdateQR();
        });
    } else {
        wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
            SERIAL_PRINTLN("AP Callback For WiFi Config");
            display.showAPQR();
        });
    }

    wifiManager.setSaveConfigCallback([&]() {
        SERIAL_PRINTLN("Save Config Callback");
        if (wifiManager.getWiFiIsSaved()) {
            SERIAL_PRINTLN(F("WiFi Saved - reset"));
        } else {
            SERIAL_PRINTLN(F("WiFi NOT Saved"));
        }
        delay(1000);
        ESP.reset();
    });

    if (!wifiManager.autoConnect(HostName.c_str(), String(SECRET_AP_PASSWORD).c_str())) {
        SERIAL_PRINTLN(F("failed to connect, we should reset and see if it connects"));
        delay(3000);
        ESP.reset();
        delay(5000);
    }

    if (forUpdate) {
        wifiManager.startConfigPortal(HostName.c_str(), String(SECRET_AP_PASSWORD).c_str());
    }
}
