extern "C"
{
#include "hw5_1.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(hw5_1)
{
    void setup()
    {
        hw5_1_create();
    }

    void teardown()
    {
        hw5_1_destroy();
    }
};

TEST(hw5_1, create)
{
    FAIL("Start here");
}

