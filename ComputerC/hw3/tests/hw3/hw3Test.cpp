extern "C"
{
#include "hw3.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(hw3)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(hw3, Return_3_to_Max_Power)
{
    int max_power = get_max_power(100);
    CHECK_EQUAL(4, max_power);
    max_power = get_max_power(200);
    CHECK_EQUAL(4, max_power);
    max_power = get_max_power(300);
    CHECK_EQUAL(5, max_power);
}

TEST(hw3, Return_Calculate_Add_Result) {
    float f = calculator(1, '+', 3);
    CHECK_EQUAL(4, f);
}

TEST(hw3, Return_Calculate_Sub_Result) {
    float f = calculator(1, '-', 3);
    CHECK_EQUAL(-2, f);
}

TEST(hw3, Return_Calculate_Mul_Result) {
    float f = calculator(2, '*', 3);
    CHECK_EQUAL(6, f);
}

TEST(hw3, Return_Calculate_Div_Result) {
    float f = calculator(2, '/', 3);
    CHECK_EQUAL(3.0/2, f);
}
