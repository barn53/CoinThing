#include "utils.h"

#if COIN_THING_SERIAL > 0
int callDepth { 0 };
uint32_t lastIndentMillis { 0 };
#endif

void formatNumber(float n, String& s, NumberFormat format, bool forceSign, bool dash00, bool compactZeroes, uint8_t forceDecimalPlaces)
{
    char buf[21];
    char buf2[21];

    uint8_t decimalPlaces(forceDecimalPlaces);
    if (forceDecimalPlaces == std::numeric_limits<uint8_t>::max()) {
        if ((n < 0 ? n * -1. : n) < 0.000001) {
            decimalPlaces = 8;
        } else if ((n < 0 ? n * -1. : n) < 0.00001) {
            decimalPlaces = 8;
        } else if ((n < 0 ? n * -1. : n) < 0.0001) {
            decimalPlaces = 7;
        } else if ((n < 0 ? n * -1. : n) < 0.001) {
            decimalPlaces = 6;
        } else if ((n < 0 ? n * -1. : n) < 0.01) {
            decimalPlaces = 6;
        } else if ((n < 0 ? n * -1. : n) < 0.1) {
            decimalPlaces = 5;
        } else if ((n < 0 ? n * -1. : n) < 1) {
            decimalPlaces = 5;
        } else {
            decimalPlaces = 2;
        }
    }

    if (forceSign && n != 0.) {
        snprintf(buf, sizeof(buf), "%+.*f", decimalPlaces, n);
    } else {
        snprintf(buf, sizeof(buf), "%.*f", decimalPlaces, n);
    }

    char thousandSeparator('#'); // means no separator
    char decimalSeparator('.');

    if (format == NumberFormat::THOUSAND_DOT_DECIMAL_COMMA) {
        thousandSeparator = '.';
        decimalSeparator = ',';
    } else if (format == NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA) {
        thousandSeparator = ' ';
        decimalSeparator = ',';
    } else if (format == NumberFormat::DECIMAL_COMMA) {
        decimalSeparator = ',';
    } else if (format == NumberFormat::THOUSAND_COMMA_DECIMAL_DOT) {
        thousandSeparator = ',';
        decimalSeparator = '.';
    } else if (format == NumberFormat::THOUSAND_BLANK_DECIMAL_DOT) {
        thousandSeparator = ' ';
        decimalSeparator = '.';
    }

    bool hasDecSep(false);
    int rev(0);
    for (auto pos = strlen(buf); pos > 0; --pos, ++rev) {
        char c = buf[pos - 1];
        if (c == '.') {
            hasDecSep = true;
        }
        buf2[rev] = c;
    }
    buf2[rev] = 0; // buf2 holds number string in reverse order

    uint8_t numTrailingZeroes(0);
    uint8_t posDecSep(0);
    if (hasDecSep) {
        for (; buf2[numTrailingZeroes] == '0'; ++numTrailingZeroes) { }
        for (posDecSep = numTrailingZeroes; buf2[posDecSep] != '.'; ++posDecSep) { }
    }

    bool reachedDecSep(false);
    uint8_t copyTo(sizeof(buf) - 1);
    uint8_t group(0);

    buf[copyTo] = 0;
    --copyTo;

    uint8_t copyFrom(0);
    if (numTrailingZeroes > 0) {
        // remove trailing zeroes (after the decimal separator)
        //  but leave at least 2 digits
        copyFrom = min<uint8_t>(numTrailingZeroes, posDecSep - 2);
    }

    // reverse copy the numeric string and apply some formatting
    for (; buf2[copyFrom] != 0 && copyTo != 0; ++copyFrom) {
        char c = buf2[copyFrom];

        if (!hasDecSep || reachedDecSep) {
            ++group;
        }

        if (c == '.') {
            c = decimalSeparator;
            reachedDecSep = true;
        }

        buf[copyTo] = c;
        --copyTo;

        if (group > 0
            && group % 3 == 0
            && buf2[copyFrom + 1] != 0
            && buf2[copyFrom + 1] != '+') {
            if (thousandSeparator != '#') {
                buf[copyTo] = thousandSeparator;
                --copyTo;
            }
        }
    }
    s = &buf[copyTo + 1];

    if (compactZeroes) {
        String pattern;
        pattern = F("0");
        pattern += decimalSeparator;
        pattern += F("00");
        uint8_t zeroesAfterDecSep(2);
        bool zeroesNotation(false);
        while (s.indexOf(pattern) != -1 && pattern.length() < s.length()) {
            pattern += F("0");
            ++zeroesAfterDecSep;
            zeroesNotation = true;
        }
        if (zeroesNotation) {
            String z(--zeroesAfterDecSep);
            z += F("z");
            pattern = pattern.substring(0, pattern.length() - 1);
            s.replace(pattern, z);
        }
    }

    s.replace(F(" "), F("\u2006")); // Six-Per-Em Space U+2006
    if (dash00) {
        String dotZero;
        dotZero = decimalSeparator;
        dotZero += F("00");
        if (s.endsWith(dotZero)) {
            String repl;
            repl = decimalSeparator;
            repl += F("\u2012"); // Figure Dash U+2012
            s.replace(dotZero, repl);
        }
    }
}

uint32_t millis_test()
{
#if 0
    uint32_t ret(millis() + (0xffffffff - 10000));
    static uint32_t last(ret);
    if (ret < last) {
        Serial.println("millis() overflow");
    }
    last = ret;
    return ret;
#else
    return millis();
#endif
}

bool doInterval(uint32_t change, uint32_t interval)
{
    if (change == 0) {
        return true;
    }
    if ((millis_test() - change) >= interval) {
        return true;
    }
    return false;
}
