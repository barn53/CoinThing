#include "utils.h"
#include <unity.h>

void setUp()
{
    // set stuff up here
}

void tearDown()
{
    // clean stuff up here
}

void loop()
{
}

void compactZeroes()
{
    String s;

    formatNumber(0.0123, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0,0123", s.c_str());

    formatNumber(0.00456, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",2z\u20064560", s.c_str());

    formatNumber(0.000789, s, NumberFormat::DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(".3z7890", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",4z\u20062580", s.c_str());

    formatNumber(0.00000147, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(".5z\u20061470", s.c_str());

    formatNumber(0.000000369, s, NumberFormat::DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",6z3690", s.c_str());

    formatNumber(0.0000003691, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, SmallDecimalNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("0.0000003691", s.c_str());

    formatNumber(0.00000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",10z\u20061200", s.c_str());

    formatNumber(0.000000004, s, NumberFormat::DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(".8z4000", s.c_str());

    formatNumber(0.0000000041, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",8z\u20064100", s.c_str());

    formatNumber(0.0000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(".9z\u20061230", s.c_str());

    formatNumber(0.00000000004, s, NumberFormat::DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",10z4000", s.c_str());

    formatNumber(0.00000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(".10z\u20061200", s.c_str());

    formatNumber(0.000000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",11z\u20061000", s.c_str());

    formatNumber(0.0000000000001234, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0.\u2012", s.c_str());

    formatNumber(-0.0000258, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("-,4z\u20062580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(".4z\u20062580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_COMMA_DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(".4z\u20062580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",4z\u20062580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(",4z2580", s.c_str());

    formatNumber(0.0000258, s, NumberFormat::DECIMAL_DOT, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING(".4z2580", s.c_str());

    formatNumber(1230., s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("1.230,00", s.c_str());
}

void forceSign()
{
    String s;

    formatNumber(0., s, NumberFormat::DECIMAL_DOT, true, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0.\u2012", s.c_str()); // no sign when zero

    formatNumber(0.0000321, s, NumberFormat::DECIMAL_DOT, true, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("+.4z3210", s.c_str());

    formatNumber(-0.000321, s, NumberFormat::DECIMAL_DOT, true, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("-.3z3210", s.c_str());

    formatNumber(0.0000321, s, NumberFormat::DECIMAL_COMMA, true, true, SmallDecimalNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("+0,0000321", s.c_str());

    formatNumber(-0.000321, s, NumberFormat::DECIMAL_DOT, true, true, SmallDecimalNumberFormat::NORMAL);
    TEST_ASSERT_EQUAL_STRING("-0.000321", s.c_str());
}

void formats()
{
    String s;

    formatNumber(0., s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0,\u2012", s.c_str());

    formatNumber(0., s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0,00", s.c_str());

    formatNumber(1., s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("1,\u2012", s.c_str());

    formatNumber(1., s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("1,00", s.c_str());

    formatNumber(0.1, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0,10", s.c_str());

    formatNumber(0.100002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0,10", s.c_str());

    formatNumber(0.10002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0,10002", s.c_str());

    formatNumber(0.1002, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("0,1002", s.c_str());

    formatNumber(38123.1299, s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("38.123,1299", s.c_str());

    formatNumber(8123987., s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, true, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("8.123.987,\u2012", s.c_str());

    formatNumber(8123987., s, NumberFormat::THOUSAND_DOT_DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("8.123.987,00", s.c_str());

    formatNumber(99999.12345678, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, false, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("99\u2006999.1235", s.c_str());

    formatNumber(100000.12345678, s, NumberFormat::THOUSAND_BLANK_DECIMAL_DOT, false, false, SmallDecimalNumberFormat::COMPACT);
    TEST_ASSERT_EQUAL_STRING("100\u2006000.12", s.c_str());
}

void forceDecimalPlaces()
{
    String s;

    formatNumber(7., s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,00", s.c_str()); // remove trailing zeroes (after the decimal separator) but leave at least 2 digits

    formatNumber(7.1, s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,10", s.c_str()); // remove trailing zeroes (after the decimal separator) but leave at least 2 digits

    formatNumber(7.11, s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,11", s.c_str()); // remove trailing zeroes (after the decimal separator) but leave at least 2 digits

    formatNumber(7.111, s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,111", s.c_str());

    formatNumber(7.1111, s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("7,111", s.c_str());

    formatNumber(0.00123, s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("0,001", s.c_str());

    formatNumber(0.000123, s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("0,00", s.c_str());

    formatNumber(-30000, s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("-30000,00", s.c_str());

    formatNumber(-30000.12399, s, NumberFormat::DECIMAL_COMMA, false, false, SmallDecimalNumberFormat::NORMAL, 3);
    TEST_ASSERT_EQUAL_STRING("-30000,124", s.c_str());
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(compactZeroes);
    RUN_TEST(forceSign);
    RUN_TEST(formats);
    RUN_TEST(forceDecimalPlaces);

    UNITY_END();
}
