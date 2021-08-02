#include "wifi_utils.h"
#include "common.h"
#include "display.h"
#include "pre.h"
#include "utils.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

extern String HostName;

#if 0
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
#endif

bool waitWiFiConnect()
{
    LOG_FUNC

    SERIAL_PRINT("Connect");
    int counter(0);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        if (counter > 50) {
            SERIAL_PRINTLN(" -");
            return false;
        }
        SERIAL_PRINT(" .");
        ++counter;
    }
    SERIAL_PRINTLN(" +");
    return true;
}

void handleWifiManager(WiFiManager& wifiManager, Display& display)
{
    LOG_FUNC

    WiFi.mode(WIFI_STA);

    wifiManager.setWiFiAutoReconnect(true);
    wifiManager.setDebugOutput(true);

    if (SPIFFS.exists(FOR_UPDATE_FILE)) {
        display.showPrepareUpdate(false);

        SERIAL_PRINTLN(F("CoinThing in update mode"));
        SERIAL_PRINT(F("Version before update: "));
        SERIAL_PRINTLN(VERSION);

        SPIFFS.remove(FOR_UPDATE_FILE);
        wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
            SERIAL_PRINTLN("AP Callback For Update");

            if (WiFi.status() != WL_CONNECTED) {
                SERIAL_PRINTLN(F("Not connected – restart"));
                display.showPrepareUpdate(true);
                delay(5000);
                ESP.restart();
            }

            File f = SPIFFS.open(VERSION_BEFORE_UPDATE_FILE, "w");
            f.print(VERSION);
            f.close();
            display.showUpdateQR();
        });

        waitWiFiConnect();
        wifiManager.startConfigPortal(HostName.c_str(), String(SECRET_AP_PASSWORD).c_str());
    } else if (SPIFFS.exists(VERSION_BEFORE_UPDATE_FILE)) {
        File f = SPIFFS.open(VERSION_BEFORE_UPDATE_FILE, "r");
        String beforeVersion(f.readString());
        f.close();
        SPIFFS.remove(VERSION_BEFORE_UPDATE_FILE);

        String currentVersion(VERSION);
        if (beforeVersion != currentVersion) {
            display.showUpdated();
        } else {
            display.showNotUpdated();
        }

        delay(2500);
        if (!waitWiFiConnect()) {
            SERIAL_PRINTLN(F("Not connected – restart"));
            ESP.restart();
        }
    } else {
        const char* menu[] = { "wifi" };
        wifiManager.setMenu(menu, 1);

        WiFi.hostname(HostName);

        wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
            SERIAL_PRINTLN(F("AP Callback For WiFi Config"));
            display.showAPQR();
        });

        wifiManager.setSaveConfigCallback([&]() {
            SERIAL_PRINTLN(F("Save Config Callback"));
            if (wifiManager.getWiFiIsSaved()) {
                SERIAL_PRINTLN(F("WiFi Saved - reset"));
            } else {
                SERIAL_PRINTLN(F("WiFi NOT Saved"));
            }
            delay(1000);
            ESP.restart();
        });

        if (!wifiManager.autoConnect(HostName.c_str(), String(SECRET_AP_PASSWORD).c_str())) {
            SERIAL_PRINTLN(F("failed to connect, we should reset and see if it connects"));
            delay(3000);
            ESP.restart();
        }
    }
}
