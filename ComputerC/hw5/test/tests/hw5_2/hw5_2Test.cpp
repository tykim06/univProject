extern "C"
{
#include "hw5_2.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(hw5_2)
{
    void setup()
    {
        hw5_2_create();
    }

    void teardown()
    {
        hw5_2_destroy();
    }
};

TEST(hw5_2, create)
{
    FAIL("Start here");
}

