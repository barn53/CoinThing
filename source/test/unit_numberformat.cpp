#include "utils.h"
#include <Arduino.h>
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

void test()
{
    String s;
    formatNumber(0.1, s, NumberFormat::THOUSAND_BLANK_DECIMAL_COMMA, false, true);
    TEST_ASSERT_EQUAL_STRING("0.100", s.c_str());
}

void setup()
{
    UNITY_BEGIN();

    RUN_TEST(test);

    UNITY_END();
}
