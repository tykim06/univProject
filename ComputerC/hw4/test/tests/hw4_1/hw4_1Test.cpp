extern "C"
{
#include "hw4_1.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(hw4_1)
{
    void setup()
    {
        hw4_1_create();
    }

    void teardown()
    {
        hw4_1_destroy();
    }
};

