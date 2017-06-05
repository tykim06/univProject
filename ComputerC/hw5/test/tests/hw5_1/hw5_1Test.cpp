extern "C"
{
#include "hw5_1.h"
}

#include "CppUTest/TestHarness.h"
#include <stdio.h>

Car_t cars[100];

TEST_GROUP(hw5_1)
{
    void setup()
    {
        hw5_1_create(cars);
    }

    void teardown()
    {
        hw5_1_destroy();
    }
};

TEST(hw5_1, add_car_when_overlap_name_return_idx) {
    Car_t car = {"k7", 2, 1};
    CHECK_EQUAL(0, add_car(cars, car));
    Car_t car1 = {"k5", 1, 5};
    CHECK_EQUAL(1, add_car(cars, car1));
}

TEST(hw5_1, add_car_when_car_name_return_idx) {
    Car_t car = {"k7", 2, 1};
    CHECK_EQUAL(0, add_car(cars, car));
    Car_t car1 = {"k5", 1, 5};
    CHECK_EQUAL(1, add_car(cars, car1));

    char name[] = "k7";
    CHECK_EQUAL(0, find_car(cars, name));
    char name1[] = "k5";
    CHECK_EQUAL(1, find_car(cars, name1));
}
