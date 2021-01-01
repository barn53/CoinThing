#include "utils.h"
#include <Arduino.h>

const char* getCurrencySymbol(const char* currency)
{
    if (strcmp(currency, "eur") == 0) {
        return "€";
    } else if (strcmp(currency, "usd") == 0) {
        return "$";
    } else {
        return currency;
    }
}
