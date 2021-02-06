#pragma once
#include <Arduino.h>

#if COIN_THING_SERIAL == 1
#define LOG_FUNC            \
    Serial.print(__func__); \
    Serial.println("()");
#else
#define LOG_FUNC
#endif

#if COIN_THING_SERIAL == 1
#define SERIAL_PRINTLN(x) Serial.println(x);
#else
#define SERIAL_PRINTLN(x)
#endif

using gecko_t = float;

String cleanUp(const String& s);

enum class NumberFormat : uint8_t {
    THOUSAND_DOT_DECIMAL_COMMA = 0, // 1.000,00
    THOUSAND_BLANK_DECIMAL_COMMA, // 1 000,00
    DECIMAL_COMMA, // 1000,00
    THOUSAND_COMMA_DECIMAL_DOT, // 1,000.00
    THOUSAND_BLANK_DECIMAL_DOT, // 1 000.00
    DECIMAL_DOT // 1000.00
};
void formatNumber(gecko_t n, String& s, NumberFormat format, bool forceSign, bool dash00, uint8_t forceDecimalPlaces = std::numeric_limits<uint8_t>::max());

struct Currency {
    const char* currency;
    const char* symbol;
};
static const Currency currencies[] = {
    { "usd", "$" }, // US Dollar
    { "eur", "€" }, // Euro
    { "btc", "₿" }, // Bitcoin - U+20BF - U+20BF
    { "eth", "Ξ" }, // Ethereum - Greek Capital Letter Xi - U+039E
    { "ltc", "Ł" }, // Latin Capital Letter L with Stroke - U+141
    { "bch", "BCH" }, // Bitcoin Cash
    { "bnb", "BNB" }, // Binance Coin
    { "eos", "EOS" }, // EOS
    { "xrp", "XRP" }, // Ripple
    { "xlm", "XLM" }, // Stellar
    { "link", "LINK" }, // Chainlink
    { "dot", "DOT" }, // Polkadot
    { "yfi", "YFI" }, // Yearn.finance
    { "aed", "AED" }, // Emirati dirham - د.إ
    { "ars", "$" }, // Argentina Peso
    { "aud", "$" }, // Australia Dollar
    { "bdt", "৳" }, // Bangladeshi taka - U+09F3 not in NotoSans
    { "bhd", "BD" }, // Bahraini dinar - .د.ب
    { "bmd", "$" }, // Bermuda Dollar
    { "brl", "R$" }, // Brazil Real
    { "cad", "$" }, // Canadian dollars
    { "chf", "CHF" }, // Swiss franc
    { "clp", "$" }, // Chilean peso
    { "cny", "¥" }, // Chinese yuan
    { "czk", "Kč" }, // Czech koruna
    { "dkk", "kr" }, // Danish krone
    { "gbp", "£" }, // Pounds Sterling
    { "hkd", "$" }, // Hong Kong dollar
    { "huf", "ft" }, // Hungarian forint
    { "idr", "Rp" }, // Indonesian rupiah
    { "ils", "₪" }, // Israeli shekel - U+20aa
    { "inr", "₹" }, // Indian rupee
    { "jpy", "¥" }, // Japanese yen
    { "krw", "₩" }, // South Korean won
    { "kwd", "KD" }, // Kuwaiti dinar -	د.ك
    { "lkr", "Rs" }, // Sri Lankan rupee
    { "mmk", "Ks" }, // Burmese kyat
    { "mxn", "$" }, // Mexican peso
    { "myr", "RM" }, // Malaysian ringgit
    { "ngn", "₦" }, // Nigerian naira
    { "nok", "kr" }, // Norwegian krone
    { "nzd", "$" }, // New Zealand dollar
    { "php", "₱" }, // Philippine peso
    { "pkr", "Rs" }, // Pakistani rupee
    { "pln", "zł" }, // Polish zloty
    { "rub", "₽" }, // Russian ruble - U+20BD
    { "sar", "SAR" }, // Saudi Arabia Riyal - ر.س
    { "sek", "krx" }, // Sweden Krona
    { "sgd", "$" }, // Singapore Dollar
    { "thb", "฿" }, // Thailand Baht - U+0E3F not in NotoSans
    { "try", "₺" }, // Turkey Lira
    { "twd", "$" }, // Taiwan New Dollar
    { "uah", "₴" }, // Ukraine Hryvnia
    { "vef", "Bs" }, // Venezuela Bolívar
    { "vnd", "₫" }, // Viet Nam Dong
    { "zar", "R" }, // South Africa Rand
    { "xdr", "XDR" }, // Special Drawing Right
    { "xag", "XAG" }, // ?
    { "xau", "XAU" } // Philadelphia Gold and Silver Index
};

bool isCurrency(const char* currency);
const char* getCurrencySymbol(const char* currency);

uint32_t millis_test();
bool doInterval(uint32_t change, uint32_t interval);
bool doChange(uint32_t change, uint32_t seen);
