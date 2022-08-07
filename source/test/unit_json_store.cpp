#include "json_store.h"
#include "unit.h"

#include <unity.h>

void testJsonStore()
{
    JsonStore secrets("secrets.json");
    TEST_ASSERT_FALSE(secrets.exists());

    secrets.set("assets", "https://raw.githubusercontent.com/barn53/assets/cointhing_v1/");
    TEST_ASSERT_TRUE(secrets.exists());
    TEST_ASSERT_TRUE(secrets.has("assets"));

    String value;
    TEST_ASSERT_TRUE(secrets.get("assets", value));
    TEST_ASSERT_EQUAL_STRING("https://raw.githubusercontent.com/barn53/assets/cointhing_v1/", value.c_str());

    TEST_ASSERT_TRUE(secrets.set("pipedream", "https://exqw7rzwu40h6xc.m.pipedream.net/"));
    TEST_ASSERT_TRUE(secrets.set("ssid", "FRITZ!Box 7590"));
    TEST_ASSERT_TRUE(secrets.set("overwrite_me", "false"));
    TEST_ASSERT_TRUE(secrets.set("remove_me", "true"));

    TEST_ASSERT_TRUE(secrets.has("assets"));
    TEST_ASSERT_TRUE(secrets.has("pipedream"));
    TEST_ASSERT_TRUE(secrets.has("ssid"));
    TEST_ASSERT_TRUE(secrets.has("overwrite_me"));
    TEST_ASSERT_TRUE(secrets.has("remove_me"));

    TEST_ASSERT_TRUE(secrets.remove("remove_me"));

    TEST_ASSERT_TRUE(secrets.has("assets"));
    TEST_ASSERT_TRUE(secrets.has("pipedream"));
    TEST_ASSERT_TRUE(secrets.has("ssid"));
    TEST_ASSERT_TRUE(secrets.has("overwrite_me"));
    TEST_ASSERT_FALSE(secrets.has("remove_me"));

    TEST_ASSERT_TRUE(secrets.set("overwrite_me", "overwritten"));
    TEST_ASSERT_TRUE(secrets.get("overwrite_me", value));
    TEST_ASSERT_EQUAL_STRING("overwritten", value.c_str());

    secrets.del();
    TEST_ASSERT_FALSE(secrets.exists());
}
