#pragma once
#include <ArduinoJson.h>

#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#else
#include <HTTPClient.h>
#include <WiFi.h>
#endif

class HttpJson {
public:
    HttpJson();

    bool read(const char* url, DynamicJsonDocument& jsonDoc);
    bool read(const char* url, DynamicJsonDocument& jsonDoc, DynamicJsonDocument& jsonFilter);

private:
    WiFiClientSecure m_client;
    HTTPClient m_http;
};
