#include "http_json.h"
#include "utils.h"
#include <StreamUtils.h>

#define HTTP_429_RESET_INTERVAL (60 * 60 * 1000)

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

    if (WiFi.isConnected()) {
        m_http.begin(m_client, url);
        m_last_http_code = m_http.GET();
        ++m_http_read_count;
        if (m_last_http_code > 0) {
            LOG_I_PRINTF("[HTTP] GET... code: %d\n", m_last_http_code);
            if (m_last_http_code == HTTP_CODE_OK) {
                ReadBufferingClient bufferedClient { m_client, 64 };

#if COIN_THING_SERIAL > 1
                ReadLoggingStream loggingStream(bufferedClient, Serial);
                deserializeJson(jsonDoc, loggingStream, DeserializationOption::Filter(jsonFilter));
                LOG_I_PRINTLN();
#else
                deserializeJson(jsonDoc, bufferedClient, DeserializationOption::Filter(jsonFilter));
#endif
                m_http.end();

                if (m_last_http_429 > 0 && (millis_test() - m_last_http_429) > HTTP_429_RESET_INTERVAL) {
                    // reset last 429 timestamp, when it is more than an hour ago
                    m_last_http_429 = 0;
                }
                return true;
            } else if (m_last_http_code == HTTP_CODE_TOO_MANY_REQUESTS) {
                m_last_http_429 = millis_test();
            }
        } else {
            LOG_I_PRINTF("[HTTP] GET... failed, error: %d - %s\n", m_last_http_code, m_http.errorToString(m_last_http_code).c_str());
        }
        m_http.end();
    } else {
        m_last_http_code = -1;
        LOG_I_PRINTLN("[HTTP] not connected");
    }
    return false;
}

bool HttpJson::read(const char* url)
{
    LOG_FUNC
    LOG_I_PRINTF("read from URL: %s\n", url);

    if (WiFi.isConnected()) {
        m_http.begin(m_client, url);
        m_last_http_code = m_http.GET();
        ++m_http_read_count;
        if (m_last_http_code > 0) {
            LOG_I_PRINTF("[HTTP] GET... code: %d\n", m_last_http_code);
            if (m_last_http_code == HTTP_CODE_OK) {
                m_http.end();
                return true;
            }
        } else {
            LOG_I_PRINTF("[HTTP] GET... failed, error: %d - %s\n", m_last_http_code, m_http.errorToString(m_last_http_code).c_str());
        }
        m_http.end();
    } else {
        m_last_http_code = -1;
        LOG_I_PRINTLN("[HTTP] not connected");
    }
    return false;
}

bool HttpJson::recentlyHTTP429() const
{
    return m_last_http_429 > 0;
}
