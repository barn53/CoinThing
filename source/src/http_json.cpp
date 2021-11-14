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
    LOG_I_PRINTF("read from URL: %s\n", url);

    m_http.begin(m_client, url);
    int httpCode = m_http.GET();
    if (httpCode > 0) {
        LOG_I_PRINTF("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            ReadBufferingClient bufferedClient { m_client, 64 };

#if COIN_THING_SERIAL > 1
            ReadLoggingStream loggingStream(bufferedClient, Serial);
            deserializeJson(jsonDoc, loggingStream, DeserializationOption::Filter(jsonFilter));
            Serial.println();
#else
            deserializeJson(jsonDoc, bufferedClient, DeserializationOption::Filter(jsonFilter));
#endif
            m_http.end();
            return true;
        }
    } else {
        LOG_I_PRINTF("[HTTP] GET... failed, error: %d - %s\n", httpCode, m_http.errorToString(httpCode).c_str());
    }
    m_http.end();
    return false;
}
