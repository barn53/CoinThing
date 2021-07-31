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

void handleWifiManager(WiFiManager& wifiManager, Display& display)
{
    LOG_FUNC

    WiFi.mode(WIFI_STA);

    wifiManager.setWiFiAutoReconnect(true);
    wifiManager.setDebugOutput(true);

    if (SPIFFS.exists(FOR_UPDATE_FILE)) {
        display.showPrepareUpdate();

        LOG_I_PRINTLN(F("CoinThing in update mode"));
        LOG_I_PRINT(F("Version before update: "));
        LOG_PRINTLN(VERSION);

        File f = SPIFFS.open(FOR_UPDATE_FILE, "r");
        String tries(f.readString());
        f.close();
        int triesI(tries.toInt());
        if (triesI < 1) {
            f = SPIFFS.open(FOR_UPDATE_FILE, "w");
            f.print(triesI + 1);
            f.close();
            LOG_I_PRINTF("This is try: %u\n", triesI);
        } else {
            LOG_I_PRINTF("Number of tries: %u - this is the last try\n", triesI);
            SPIFFS.remove(FOR_UPDATE_FILE);
        }

        wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
            LOG_I_PRINTLN("AP Callback For Update");

            if (!WiFi.isConnected()) {
                LOG_I_PRINTLN(F("Not connected – restart"));
                if (!SPIFFS.exists(FOR_UPDATE_FILE)) { // was last try and failed again
                    display.showPrepareUpdateFailed();
                    delay(5000);
                }
                ESP.restart();
            }

            f = SPIFFS.open(VERSION_BEFORE_UPDATE_FILE, "w");
            f.print(VERSION);
            f.close();
            SPIFFS.remove(FOR_UPDATE_FILE);
            display.showUpdateQR();
        });

        delay(1000);
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
        if (!WiFi.isConnected()) {
            LOG_I_PRINTLN(F("Not connected – restart"));
            ESP.restart();
        }
    } else {
        const char* menu[] = { "wifi" };
        wifiManager.setMenu(menu, 1);

        WiFi.hostname(HostName);

        wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
            LOG_I_PRINTLN(F("AP Callback For WiFi Config"));
            display.showAPQR();
        });

        wifiManager.setSaveConfigCallback([&]() {
            LOG_I_PRINTLN(F("Save Config Callback"));
            if (wifiManager.getWiFiIsSaved()) {
                LOG_I_PRINTLN(F("WiFi Saved - reset"));
            } else {
                LOG_I_PRINTLN(F("WiFi NOT Saved"));
            }
            delay(1000);
            ESP.restart();
        });

        if (!wifiManager.autoConnect(HostName.c_str(), String(SECRET_AP_PASSWORD).c_str())) {
            LOG_I_PRINTLN(F("failed to connect, we should reset and see if it connects"));
            delay(3000);
            ESP.restart();
            delay(5000);
        }
    }
}
