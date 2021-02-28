#pragma once

class WiFiManager;
class Display;

void wifiSleep();
void wifiWake();
void setupWiFi();

void handleWifiManager(WiFiManager& wifiManager, Display& display);
