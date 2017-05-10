extern "C"
{
#include "hw3_1.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(hw3_1)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(hw3_1, Return_3_to_Max_Power)
{
    int max_power = get_max_power(100);
    CHECK_EQUAL(4, max_power);
    max_power = get_max_power(200);
    CHECK_EQUAL(4, max_power);
    max_power = get_max_power(300);
    CHECK_EQUAL(5, max_power);
}
