extern "C"
{
#include "string.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(string)
{
    void setup()
    {
        string_create();
    }

    void teardown()
    {
        string_destroy();
    }
};

TEST(string, charToIndex_whenNoSmallLetter_returnNagative1)
{
    CHECK_EQUAL(char_to_index('A'), -1);
}

TEST(string, charToIndex_whenSmallLetter_returnIndex)
{
    CHECK_EQUAL(1, char_to_index('a'));
    CHECK_EQUAL(2, char_to_index('b'));
    CHECK_EQUAL(3, char_to_index('c'));
}
