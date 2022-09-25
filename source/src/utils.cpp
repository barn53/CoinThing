#include "utils.h"
#include "b64.h"
#include "common.h"
#include "json_store.h"

#include <AES.h>

#if COIN_THING_SERIAL > 0
int callDepth { 0 };
uint32_t lastIndentMillis { 0 };
#endif

extern JsonStore xSecrets;

#define AES_KEY_SIZE 16
#define AES_BLOCK_SIZE 16
#define AES_VALUE_SIZE 32

String urlencode(const String& url)
{
    String encoded = "";
    char c;
    char code0;
    char code1;
    for (auto i = 0; i < url.length(); ++i) {
        c = url.charAt(i);
        if (c == ' ') {
            encoded += '+';
        } else if (isalnum(c)) {
            encoded += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
        yield();
    }
    return encoded;
}

void hex(const uint8_t* buffer, size_t length)
{
    Serial.println("\n--------------------------------------------------------------------------");
    Serial.printf("[HEXDUMP] Address: %p len: 0x%X (%d)\n", buffer, length, length);
    for (size_t row = 0; row * 8 < length; ++row) {
        for (size_t col = 0; col < 8; ++col) {
            size_t idx(row * 8 + col);
            Serial.printf("%02x", buffer[idx]);
            if (idx == length - 1) {
                Serial.print(" | ");
            } else {
                Serial.print("   ");
            }
        }

        Serial.print("    ");
        for (size_t col = 0; col < 8; ++col) {
            size_t idx(row * 8 + col);
            uint8_t c = buffer[idx];
            if (c == 0x0a || c == 0x0d) {
                c = ' ';
            }
            Serial.printf("%c", c);
            if (idx == length - 1) {
                Serial.print(" | ");
            } else {
                Serial.print("   ");
            }
        }
        Serial.println();
    }
    Serial.println("--------------------------------------------------------------------------");
}

void copyStringToSizedBuffer(uint8_t* buffer, const String& s, size_t length)
{
    memset(buffer, 0, length);
    for (size_t ii = 0; ii < s.length() && ii < length; ++ii) {
        buffer[ii] = s.charAt(ii);
    }
}

void getAes128Key(uint8_t* key)
{
#ifndef UNIT_TEST
    String s;
    xSecrets.get(F("aes128key"), s);
#else
    String s("abcdefghijklmnop");
#endif
    copyStringToSizedBuffer(key, s, AES_KEY_SIZE);
}

String encryptEncode(const String& value)
{
    uint8_t key[AES_KEY_SIZE];
    getAes128Key(key);

    AES128 aes128;
    aes128.setKey(key, AES_KEY_SIZE);

    uint8_t val32[AES_VALUE_SIZE];
    copyStringToSizedBuffer(val32, value, AES_VALUE_SIZE);

    uint8_t cypher32[AES_VALUE_SIZE];
    aes128.encryptBlock(&cypher32[0], &(val32[0]));
    aes128.encryptBlock(&cypher32[AES_BLOCK_SIZE], &(val32[AES_BLOCK_SIZE]));

    int encodedLength = b64::encodedLength(AES_VALUE_SIZE);
    char encoded[encodedLength];
    b64::encode(encoded, reinterpret_cast<char*>(cypher32), AES_VALUE_SIZE);

    return encoded;
}

String decodeDecrypt(const String& value)
{
    int decodedLength = b64::decodedLength(const_cast<char*>(value.c_str()), value.length());
    char decoded[decodedLength];
    b64::decode(decoded, const_cast<char*>(value.c_str()), value.length());

    uint8_t key[AES_KEY_SIZE];
    getAes128Key(key);

    AES128 aes128;
    aes128.setKey(key, AES_KEY_SIZE);

    uint8_t cypher32[AES_VALUE_SIZE];
    copyStringToSizedBuffer(cypher32, decoded, AES_VALUE_SIZE);

    uint8_t decrypted32[AES_VALUE_SIZE + 1];
    aes128.decryptBlock(&decrypted32[0], &(cypher32[0]));
    aes128.decryptBlock(&decrypted32[AES_BLOCK_SIZE], &(cypher32[AES_BLOCK_SIZE]));
    decrypted32[AES_VALUE_SIZE] = 0;

    return String(reinterpret_cast<char*>(decrypted32));
}

void formatNumber(gecko_t n, String& s, NumberFormat format, bool forceSign, bool dash00, SmallDecimalNumberFormat smallDecimalNumberFormat, uint8_t forceDecimalPlaces)
{
    char buf[21];
    char buf2[21];

    uint8_t decimalPlaces(forceDecimalPlaces);
    gecko_t absoluteValue((n < 0 ? n * -1. : n));

    if (forceDecimalPlaces == std::numeric_limits<uint8_t>::max()) {
        decimalPlaces = 4;
        if (absoluteValue < 0.00000001) {
            decimalPlaces = 12;
        } else if (absoluteValue < 0.0000001) {
            decimalPlaces = 11;
        } else if (absoluteValue < 0.000001) {
            decimalPlaces = 10;
        } else if (absoluteValue < 0.00001) {
            decimalPlaces = 9;
        } else if (absoluteValue < 0.0001) {
            decimalPlaces = 8;
        } else if (absoluteValue < 0.001) {
            decimalPlaces = 7;
        } else if (absoluteValue < 0.01) {
            decimalPlaces = 6;
        } else if (absoluteValue < 1.) {
            decimalPlaces = 5;
        } else if (static_cast<uint32_t>(absoluteValue) > 99999) {
            decimalPlaces = 2;
        }
    }

    String largeNumberPostfix;
    if (n >= 1000000000000) { // trillion
        n /= 1000000000000.;
        decimalPlaces = 3;
        largeNumberPostfix = 'T';
    } else if (n >= 1000000000) { // billion
        n /= 1000000000.;
        decimalPlaces = 3;
        largeNumberPostfix = 'B';
    } else if (n >= 1000000) { // million
        n /= 1000000.;
        decimalPlaces = 3;
        largeNumberPostfix = 'M';
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

    if (smallDecimalNumberFormat == SmallDecimalNumberFormat::COMPACT
        && n != 0.
        && absoluteValue < 0.01) {
        String pattern;
        String sign;
        if (s[0] == '+' || s[0] == '-') {
            sign = s[0];
            pattern = s[0];
        }
        pattern += F("0");
        pattern += decimalSeparator;
        pattern += F("00");
        uint8_t zeroesAfterDecSep(2);
        bool compactNotation(false);
        while (s.indexOf(pattern) == 0 && pattern.length() < s.length()) {
            pattern += F("0");
            ++zeroesAfterDecSep;
            compactNotation = true;
        }
        if (compactNotation) {
            String z(sign);
            z += decimalSeparator;
            z += (--zeroesAfterDecSep);
            z += F("z");
            if (thousandSeparator != '#') {
                z += F(" ");
            }
            pattern = pattern.substring(0, pattern.length() - 1);
            s.replace(pattern, z);

            // s = "+.4z 123" --> "+.4z 1230"
            // 4: fix number of digits after z
            uint8_t rpad(4 - (s.length() - z.length()));
            for (; rpad > 0; --rpad) {
                s += '0';
            }
        }
    }

    if (dash00) {
        String dotZero(decimalSeparator);
        dotZero += F("00");
        if (s.endsWith(dotZero)) {
            String repl;
            repl = decimalSeparator;
            repl += F("\u2012"); // Figure Dash U+2012
            s.replace(dotZero, repl);
        }
    }

    if (!largeNumberPostfix.isEmpty()) {
        if (thousandSeparator != '#') {
            s += F(" ");
        }
        s += largeNumberPostfix;
    }

    s.replace(F(" "), F("\u2006")); // Six-Per-Em Space U+2006
}

// format large numbers:
//   https://www.antidote.info/en/blog/reports/millions-billions-and-other-large-numbers

void addCurrencySmbol(String& value, const String& symbol, CurrencySymbolPosition position)
{
    if (position == CurrencySymbolPosition::LEADING) {
        value = symbol + value;
    } else {
        value += symbol;
    }
}

uint32_t millis_test()
{
#if 0
    uint32_t ret(millis() + (0xffffffff - 10000));
    static uint32_t last(ret);
    if (ret < last) {
        LOG_PRINTLN("millis() overflow");
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
