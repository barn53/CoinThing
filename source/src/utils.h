#pragma once
#include <Arduino.h>

#if COIN_THING_SERIAL > 0
#define SERIAL_PRINT(x) Serial.print(x);
#define SERIAL_PRINTLN(x) Serial.println(x);
#define FILE_LINE           \
    Serial.print(__FILE__); \
    Serial.print(":");      \
    Serial.println(__LINE__);
#else
#define SERIAL_PRINT(x)
#define SERIAL_PRINTLN(x)
#define FILE_LINE
#endif

#if COIN_THING_SERIAL > 0
#define LOG_FUNC                       \
    Serial.print("-> ");               \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(" @ ");               \
    FILE_LINE
#else
#define LOG_FUNC
#endif

using gecko_t = float;

enum class NumberFormat : uint8_t {
    THOUSAND_DOT_DECIMAL_COMMA = 0, // 1.000,00
    THOUSAND_BLANK_DECIMAL_COMMA, // 1 000,00
    DECIMAL_COMMA, // 1000,00
    THOUSAND_COMMA_DECIMAL_DOT, // 1,000.00
    THOUSAND_BLANK_DECIMAL_DOT, // 1 000.00
    DECIMAL_DOT // 1000.00
};
void formatNumber(gecko_t n, String& s, NumberFormat format, bool forceSign, bool dash00, uint8_t forceDecimalPlaces = std::numeric_limits<uint8_t>::max());

uint32_t millis_test();
bool doInterval(uint32_t change, uint32_t interval);
