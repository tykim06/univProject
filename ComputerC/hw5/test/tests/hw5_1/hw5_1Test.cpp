extern "C"
{
#include "hw5_1.h"
}

#include "CppUTest/TestHarness.h"
#include <stdio.h>

Car_t car[100];

TEST_GROUP(hw5_1)
{
    void setup()
    {
        hw5_1_create(car);
    }

    void teardown()
    {
        hw5_1_destroy();
    }
};

TEST(hw5_1, add_car_when_overlap_name_return_idx) {
    printf("add_car\n");

    CHECK_EQUAL(0, add_car(car));
    CHECK_EQUAL(1, add_car(car));
}

TEST(hw5_1, add_car_when_car_name_return_idx) {
    printf("find_car\n");

    CHECK_EQUAL(0, add_car(car));
    CHECK_EQUAL(1, add_car(car));

    CHECK_EQUAL(0, find_car(car));
    CHECK_EQUAL(1, find_car(car));
}
