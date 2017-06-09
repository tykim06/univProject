extern "C"
{
#include "hw4_3.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(hw4_3)
{
    void setup()
    {
        hw4_3_create();
    }

    void teardown()
    {
        hw4_3_destroy();
    }
};

TEST(hw4_3, count_digit_when_N_return_counted_int)
{
    int counted_digit[] = {0,1,1,1,1,1,1,1,1,1};
    MEMCMP_EQUAL(counted_digit, count_digit(123456789), 10);
}

TEST(hw4_3, sum_digit_when_N_return_sum) {
    CHECK_EQUAL(45, sum_digit(123456789));
}

TEST(hw4_3, add_digital_when_N_return_sum) {
    CHECK_EQUAL(9, add_digital(123456789));
}
