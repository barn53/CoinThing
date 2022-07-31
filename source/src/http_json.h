#pragma once
#include "utils.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

class HttpJson {
public:
    HttpJson();

    bool read(const char* url, DynamicJsonDocument& jsonDoc);
    bool read(const char* url, DynamicJsonDocument& jsonDoc, DynamicJsonDocument& jsonFilter);

    int getLastHttpCode() const { return m_last_http_code; }
    size_t getHttpCount() const { return m_http_read_count; }
    uint32_t getLastHTTP429() const { return m_last_http_429; }

private:
    WiFiClientSecure m_client;
    HTTPClient m_http;
    int m_last_http_code { 0 };
    size_t m_http_read_count { 0 };
    uint32_t m_last_http_429 { 0 };
};
