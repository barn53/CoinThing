#include "unit.h"
#include "utils.h"

#include <unity.h>

void endeCrypt(const String& value, const char* expect = nullptr)
{
    String encrypted(encryptEncode(value));
    String decrypted(decodeDecrypt(encrypted));

    if (expect == nullptr) {
        TEST_ASSERT_EQUAL_STRING(value.c_str(), decrypted.c_str());
    } else {
        TEST_ASSERT_EQUAL_STRING(expect, decrypted.c_str());
    }
}

void testCrypt()
{
    endeCrypt("abcdefghijklmnopqrstuvwxyz");
    endeCrypt("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    endeCrypt("0123456789");
    endeCrypt("");
    endeCrypt("#");
    endeCrypt("abcdefghijklmnopqrstuvwxyz123456"); // exactly 32 bytes
    endeCrypt("abcdefghijklmnopqrstuvwxyz1234567890", "abcdefghijklmnopqrstuvwxyz123456");
}
