#include "wifi_utils.h"
#include "common.h"
#include "display.h"
#include "json_store.h"
#include "pre.h"
#include "utils.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

extern String HostName;
extern JsonStore Secrets;

#define JSON_DOCUMENT_WIFI_SIZE 128

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

bool waitWiFiConnect()
{
    LOG_FUNC

    LOG_I_PRINT("Connecting ");
    int counter(0);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        if (counter > 50) {
            LOG_PRINTLN(" - failed");
            return false;
        }
        LOG_PRINT(".");
        ++counter;
    }
    LOG_PRINTLN(" - success");

    LOG_I_PRINTF("\nConnected\n IP address: %s\n", WiFi.localIP().toString().c_str());
    LOG_I_PRINTF(" Hostname: %s\n", WiFi.hostname().c_str());
    LOG_I_PRINTF(" MAC: %s\n", WiFi.macAddress().c_str());
    return true;
}

bool setupWiFi(const char* ssid, const char* pwd)
{
    wifiWake();

    WiFi.hostname(HostName);
    WiFi.begin(ssid, pwd);

    return waitWiFiConnect();
}

void handleWifi(Display& display)
{
    LOG_FUNC

    bool successFromWiFiSecret(false);
    if (Secrets.has(F("ssid"))) {
        LOG_I_PRINTLN(F("WhaleTicker WiFi initialization from /secrets.json file"));

        String ssid;
        String pwd;
        if (Secrets.get(F("ssid"), ssid) && Secrets.get(F("pwd"), pwd)) {
            successFromWiFiSecret = setupWiFi(ssid.c_str(), pwd.c_str());
        } else {
            LOG_I_PRINTLN("Secrets access failed for ssid and pwd");
        }
    }

    WiFiManager wifiManager;

    WiFi.mode(WIFI_STA);

    wifiManager.setWiFiAutoReconnect(true);
    wifiManager.setDebugOutput(true);

    if (SPIFFS.exists(FOR_UPDATE_FILE)) {
        display.showPrepareUpdate(false);

        LOG_I_PRINTLN(F("CoinThing in update mode"));
        LOG_I_PRINT(F("Version before update: "));
        LOG_PRINTLN(VERSION);

        SPIFFS.remove(FOR_UPDATE_FILE);
        wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
            LOG_I_PRINTLN("AP Callback For Update");

            if (WiFi.status() != WL_CONNECTED) {
                LOG_I_PRINTLN(F("Not connected – restart"));
                display.showPrepareUpdate(true);
                delay(5000);
                ESP.restart();
            }

            File f = SPIFFS.open(VERSION_BEFORE_UPDATE_FILE, "w");
            f.print(VERSION);
            f.close();
            Settings::handlePowerupSequenceForResetEnd();
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
            LOG_I_PRINTLN(F("Not connected – restart"));
            ESP.restart();
        }
    } else if (!successFromWiFiSecret) {
        const char* menu[] = { "wifi" };
        wifiManager.setMenu(menu, 1);

        WiFi.hostname(HostName);

        wifiManager.setAPCallback([&](WiFiManager* wifiManager) {
            LOG_I_PRINTLN(F("AP Callback For WiFi Config"));
            Settings::handlePowerupSequenceForResetEnd();
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
        }
    }
}
