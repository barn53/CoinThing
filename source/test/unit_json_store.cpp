#include "json_store.h"
#include "unit.h"

#include <unity.h>

void testJsonStore()
{
    JsonStore unit_test("unit_test.json");
    unit_test.deleteStore();
    TEST_ASSERT_FALSE(unit_test.exists());

    unit_test.set("assets", "https://raw.githubusercontent.com/");
    TEST_ASSERT_TRUE(unit_test.exists());
    TEST_ASSERT_TRUE(unit_test.has("assets"));

    String value;
    TEST_ASSERT_TRUE(unit_test.get("assets", value));
    TEST_ASSERT_EQUAL_STRING("https://raw.githubusercontent.com/", value.c_str());

    TEST_ASSERT_TRUE(unit_test.set("pipedream", "https://exqw7rzwu40h6xc.m.pipedream.net/"));
    TEST_ASSERT_TRUE(unit_test.set("ssid", "FRITZ!Box 7590"));
    TEST_ASSERT_TRUE(unit_test.set("overwrite_me", "false"));
    TEST_ASSERT_TRUE(unit_test.set("remove_me", "true"));

    TEST_ASSERT_TRUE(unit_test.has("assets"));
    TEST_ASSERT_TRUE(unit_test.has("pipedream"));
    TEST_ASSERT_TRUE(unit_test.has("ssid"));
    TEST_ASSERT_TRUE(unit_test.has("overwrite_me"));
    TEST_ASSERT_TRUE(unit_test.has("remove_me"));

    TEST_ASSERT_TRUE(unit_test.remove("remove_me"));

    TEST_ASSERT_TRUE(unit_test.has("assets"));
    TEST_ASSERT_TRUE(unit_test.has("pipedream"));
    TEST_ASSERT_TRUE(unit_test.has("ssid"));
    TEST_ASSERT_TRUE(unit_test.has("overwrite_me"));
    TEST_ASSERT_FALSE(unit_test.has("remove_me"));

    TEST_ASSERT_TRUE(unit_test.set("overwrite_me", "overwritten"));
    TEST_ASSERT_TRUE(unit_test.get("overwrite_me", value));
    TEST_ASSERT_EQUAL_STRING("overwritten", value.c_str());

    unit_test.deleteStore();
    TEST_ASSERT_FALSE(unit_test.exists());
}
