#include "unit.h"
#include "utils.h"

#include <unity.h>

void compactZeroes()
{
    String s;

    formatNumber(0.0123, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0,0123", s.c_str());

    formatNumber(0.01234567, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0,01235", s.c_str());

    formatNumber(0.00456, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",2z\u20064560", s.c_str());

    formatNumber(0.000789, s, NumberFormat::DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(".3z7890", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",4z\u20062580", s.c_str());

    formatNumber(0.00000147, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(".5z\u20061470", s.c_str());

    formatNumber(0.000000369, s, NumberFormat::DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",6z3690", s.c_str());

    formatNumber(0.0000003691, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0.0000003691", s.c_str());

    formatNumber(0.00000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",10z\u20061230", s.c_str());

    formatNumber(0.000000004, s, NumberFormat::DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(".8z4000", s.c_str());

    formatNumber(0.0000000041, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",8z\u20064100", s.c_str());

    formatNumber(0.0000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(".9z\u20061234", s.c_str());

    formatNumber(0.00000000004, s, NumberFormat::DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",10z4000", s.c_str());

    formatNumber(0.00000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(".10z\u20061230", s.c_str());

    formatNumber(0.000000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",11z\u20061200", s.c_str());

    formatNumber(0.0000000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",12z\u20061000", s.c_str());

    formatNumber(0.00000000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING("0.\u2012", s.c_str());

    formatNumber(-0.0000258, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING("-,4z\u20062580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(".4z\u20062580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_COMMA_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(".4z\u20062580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",4z\u20062580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(",4z2580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING(".4z2580", s.c_str());

    formatNumber(1230., s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, false, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING("1.230,00", s.c_str());
}

void compactZeroesLessDecimalPlaces()
{
    String s;

    formatNumber(0.0123, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0,012", s.c_str());

    formatNumber(0.01234567, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0,012", s.c_str());

    formatNumber(0.00456, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",2z\u2006456", s.c_str());

    formatNumber(0.0007893, s, NumberFormat::DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(".3z789", s.c_str());

    formatNumber(0.00002582, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",4z\u2006258", s.c_str());

    formatNumber(0.000001475, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(".5z\u2006148", s.c_str());

    formatNumber(0.0000003695, s, NumberFormat::DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",6z370", s.c_str());

    formatNumber(0.0000003691, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0.00000037", s.c_str());

    formatNumber(0.00000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",10z\u2006120", s.c_str());

    formatNumber(0.000000004, s, NumberFormat::DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(".8z400", s.c_str());

    formatNumber(0.0000000041, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",8z\u2006410", s.c_str());

    formatNumber(0.0000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(".9z\u2006123", s.c_str());

    formatNumber(0.00000000004, s, NumberFormat::DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",10z400", s.c_str());

    formatNumber(0.00000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(".10z\u2006120", s.c_str());

    formatNumber(0.000000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",11z\u2006100", s.c_str());

    formatNumber(0.0000000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0.\u2012", s.c_str());

    formatNumber(-0.0000258, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("-,4z\u2006258", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(".4z\u2006258", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_COMMA_DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(".4z\u2006258", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",4z\u2006258", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::DECIMAL_COMMA, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(",4z258", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::DECIMAL_DOT, false, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING(".4z258", s.c_str());
}

void forceSign()
{
    String s;

    formatNumber(0., s, NumberFormat::DECIMAL_DOT, true, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING("0.\u2012", s.c_str()); // no sign when zero

    formatNumber(0.0000321, s, NumberFormat::DECIMAL_DOT, true, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING("+.4z3210", s.c_str());

    formatNumber(-0.000321, s, NumberFormat::DECIMAL_DOT, true, true, CompressNumberFormat::COMPACT_SMALL);
    TEST_ASSERT_EQUAL_STRING("-.3z3210", s.c_str());

    formatNumber(0.0000321, s, NumberFormat::DECIMAL_COMMA, true, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("+0,0000321", s.c_str());

    formatNumber(-0.000321, s, NumberFormat::DECIMAL_DOT, true, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("-0.000321", s.c_str());
}

void forceSignLessDecimalPlaces()
{
    String s;

    formatNumber(0., s, NumberFormat::DECIMAL_DOT, true, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0.\u2012", s.c_str()); // no sign when zero

    formatNumber(0.0000321, s, NumberFormat::DECIMAL_DOT, true, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("+.4z321", s.c_str());

    formatNumber(-0.000321, s, NumberFormat::DECIMAL_DOT, true, true, CompressNumberFormat::COMPACT_SMALL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("-.3z321", s.c_str());

    formatNumber(0.0000321, s, NumberFormat::DECIMAL_COMMA, true, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("+0,000032", s.c_str());

    formatNumber(-0.000321, s, NumberFormat::DECIMAL_DOT, true, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("-0.00032", s.c_str());
}

void formats()
{
    String s;

    formatNumber(0., s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0,\u2012", s.c_str());

    formatNumber(0., s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0,00", s.c_str());

    formatNumber(1., s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("1,\u2012", s.c_str());

    formatNumber(1., s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("1,00", s.c_str());

    formatNumber(0.1, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0,10", s.c_str());

    formatNumber(0.100002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0,10", s.c_str());

    formatNumber(0.10002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0,10002", s.c_str());

    formatNumber(0.1002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0,1002", s.c_str());

    formatNumber(38123.1299, s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("38.123,1299", s.c_str());

    formatNumber(8123987., s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("8.123.987,\u2012", s.c_str());

    formatNumber(8123987., s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("8.123.987,00", s.c_str());

    formatNumber(99999.12345678, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, false, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("99\u2006999.1235", s.c_str());

    formatNumber(100000.12345678, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, false, CompressNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("100\u2006000.12", s.c_str());
}

void formatsLessDecimalPlaces()
{
    String s;

    formatNumber(0.12345, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0,123", s.c_str());

    formatNumber(0.100002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0,10", s.c_str());

    formatNumber(0.10002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0,10", s.c_str());

    formatNumber(0.1002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0,10", s.c_str());

    formatNumber(0.102, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("0,102", s.c_str());

    formatNumber(1.12345, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("1,12", s.c_str());

    formatNumber(1000.12345, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("1\u2006000,12", s.c_str());

    formatNumber(38123.1299, s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("38.123,13", s.c_str());

    formatNumber(8123987., s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("8.123.987,\u2012", s.c_str());

    formatNumber(8123987., s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("8.123.987,00", s.c_str());

    formatNumber(99999.12345678, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, false, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("99\u2006999.12", s.c_str());

    formatNumber(100000.12345678, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, false, CompressNumberFormat::NORMAL, std::numeric_limits<uint8_t>::max(), true);
    TEST_ASSERT_EQUAL_STRING("100\u2006000.12", s.c_str());
}

void forceDecimalPlaces()
{
    String s;

    formatNumber(7., s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,00", s.c_str()); // remove trailing zeroes (after the decimal separator) but leave at least 2 digits

    formatNumber(7.1, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,10", s.c_str()); // remove trailing zeroes (after the decimal separator) but leave at least 2 digits

    formatNumber(7.11, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,11", s.c_str()); // remove trailing zeroes (after the decimal separator) but leave at least 2 digits

    formatNumber(7.111, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,111", s.c_str());

    formatNumber(7.1111, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,111", s.c_str());

    formatNumber(0.00123, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("0,001", s.c_str());

    formatNumber(0.000123, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("0,00", s.c_str());

    formatNumber(-30000, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("-30000,00", s.c_str());

    formatNumber(-30000.12399, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("-30000,124", s.c_str());
}

void largeNumbers()
{
    String s;

    formatNumber(1000000, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::COMPACT_LARGE);
    TEST_ASSERT_EQUAL_STRING("1,000M", s.c_str());

    formatNumber(-1000000, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::COMPACT_LARGE);
    TEST_ASSERT_EQUAL_STRING("-1,000M", s.c_str());

    formatNumber(123456789, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::COMPACT_LARGE);
    TEST_ASSERT_EQUAL_STRING("123,457M", s.c_str());

    formatNumber(1234567890, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::COMPACT_LARGE);
    TEST_ASSERT_EQUAL_STRING("1,235B", s.c_str());

    formatNumber(1234567890123, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::COMPACT_LARGE);
    TEST_ASSERT_EQUAL_STRING("1,235T", s.c_str());

    formatNumber(123456789012345, s, NumberFormat::DECIMAL_COMMA, false, false, CompressNumberFormat::COMPACT_LARGE);
    TEST_ASSERT_EQUAL_STRING("123,457T", s.c_str());

    formatNumber(1234567890123456, s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, false, CompressNumberFormat::COMPACT_LARGE);
    TEST_ASSERT_EQUAL_STRING("1.234,568\u2006T", s.c_str());
}

void uglyRounding()
{
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, R"({"values":[2503.67, 2503.68, 2503.69, 2503.66, 2504.1, 2511.13]})");
    const char* expected[] = { "2503,67", "2503,68", "2503,69", "2503,66", "2504,10", "2511,13" };
    // with ArduinoJson version 6.18 it was (using float as default): "2503,6699", "2503,6799", "2503,6899", "2503,6599", "2504,1001", "2511,1299" };
    String s;
    JsonArray values = doc["values"];
    for (size_t ii = 0; ii < values.size(); ++ii) {
        gecko_t value = values[ii];
        formatNumber(value, s, NumberFormat::DECIMAL_COMMA, false, true, CompressNumberFormat::NORMAL);
        TEST_ASSERT_EQUAL_STRING(expected[ii], s.c_str());
    }
}
