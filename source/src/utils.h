#pragma once
#include <Arduino.h>

String cleanUp(const String& s);

enum class NumberFormat : uint8_t {
    THOUSAND_DOT_DECIMAL_COMMA = 0, // 1.000,00
    THOUSAND_BLANK_DECIMAL_COMMA, // 1 000,00
    DECIMAL_COMMA, // 1000,00
    THOUSAND_COMMA_DECIMAL_DOT, // 1,000.00
    THOUSAND_BLANK_DECIMAL_DOT, // 1 000.00
    DECIMAL_DOT // 1000.00
};
void formatNumber(double n, String& s, NumberFormat format, bool forceSign, bool dash00, uint8_t forceDecimalPlaces = std::numeric_limits<uint8_t>::max());

struct Currency {
    const char* currency;
    const char* symbol;
};
static const Currency currencies[] = { { "USD", "$" }, { "EUR", "€" } };

const char* getCurrencySymbol(const char* currency);
