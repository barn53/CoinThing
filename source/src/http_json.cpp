#include "http_json.h"
#include "utils.h"
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
    LOG_FUNC

#if COIN_THING_SERIAL > 0
    Serial.printf("read from URL: %s\n", url);
#endif

    m_http.begin(m_client, url);
    int httpCode = m_http.GET();
    if (httpCode > 0) {
#if COIN_THING_SERIAL > 0
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
#endif
        if (httpCode == HTTP_CODE_OK) {
#if COIN_THING_SERIAL > 1
            ReadLoggingStream loggingStream(m_client, Serial);
            deserializeJson(jsonDoc, loggingStream, DeserializationOption::Filter(jsonFilter));
            Serial.println();
#else
            deserializeJson(jsonDoc, m_client, DeserializationOption::Filter(jsonFilter));
#endif
            return true;
        }
    } else {
#if COIN_THING_SERIAL > 0
        Serial.printf("[HTTP] GET... failed, error: %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
#endif
    }
    return false;
}
