extern "C"
{
#include "hw4_2.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(hw4_2)
{
    void setup()
    {
        hw4_2_create();
    }

    void teardown()
    {
        hw4_2_destroy();
    }
};

TEST(hw4_2, get_p_switched_case_when_null_expect_null)
{
    STRCMP_EQUAL(NULL, get_p_switched_case(NULL, NULL));
}

TEST(hw4_2, get_p_switched_case_when_str_shorter_than_find_expect_null) {
    char str[] = "sejong";
    char find[] = "find sejong";

    STRCMP_EQUAL(NULL, get_p_switched_case(str, find));
}

TEST(hw4_2, get_p_switched_case_when_find_has_space_expect_null) {
    char str[] = "i love sejong";
    char find[] = "love sejong";

    STRCMP_EQUAL(NULL, get_p_switched_case(str, find));
}

TEST(hw4_2, get_p_switched_case_when_included_word_expect_word) {
    char str[] = "i LoVe SeJong";
    char find[] = "sEjoNg";

    STRCMP_EQUAL("sEjONG", get_p_switched_case(str, find));
}

TEST(hw4_2, get_p_switched_case_when_included_word_expect_word2) {
    char str[] = "I Love sejong";
    char find[] = "love";

    STRCMP_EQUAL("lOVE", get_p_switched_case(str, find));
}
