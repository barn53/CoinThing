#include "unit.h"

#include <unity.h>
#define FS_NO_GLOBALS
#include <FS.h>

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

void setup()
{
    SPIFFS.begin();
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(compactZeroes);
    RUN_TEST(forceSign);
    RUN_TEST(formats);
    RUN_TEST(forceDecimalPlaces);
    RUN_TEST(uglyRounding);

    RUN_TEST(testJsonStore);

    UNITY_END();
}
