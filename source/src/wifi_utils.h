#pragma once
#include <WiFiManager.h>

class Display;

void wifiSleep();
void wifiWake();
void setupWiFi();

void handleWifiManager(WiFiManager& wifiManager, Display& display);
