#include "http.h"
#include <Arduino.h>
#include <StreamUtils.h>

HttpJson::HttpJson()
{
    m_client.setInsecure();

    m_http.useHTTP10(true); // stream is only available with HTTP1.0 (no chunked transfer)
    m_http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
}

bool HttpJson::read(const char* url, DynamicJsonDocument& jsonDoc)
{
    DynamicJsonDocument jsonFilter(0);
    jsonFilter.set(true);

    return read(url, jsonDoc, jsonFilter);
}

bool HttpJson::read(const char* url, DynamicJsonDocument& jsonDoc, DynamicJsonDocument& jsonFilter)
{
    Serial.printf("read from URL: %s\n", url);

    m_http.begin(m_client, url);
    int httpCode = m_http.GET();
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            ReadLoggingStream loggingStream(m_client, Serial);
            deserializeJson(jsonDoc, loggingStream, DeserializationOption::Filter(jsonFilter));
            Serial.println();
            return true;
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
    }
    return false;
}
