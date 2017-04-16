extern "C"
{
#include "square.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(square)
{
    void setup()
    {
        square_create();
    }

    void teardown()
    {
        square_destroy();
    }
};

TEST(square, getArea3DSquare_whenAnyOfNagative_retunNagative1)
{
    CHECK_EQUAL(-1, get_area_3Dsquare(-1, 6.3, 2.4));
    CHECK_EQUAL(-1, get_area_3Dsquare(6.3, -1, 2.4));
    CHECK_EQUAL(-1, get_area_3Dsquare(6.3, 3.3, -1));
}

TEST(square, getArea3dSquare_returnArea)
{
    CHECK_EQUAL((int)((2*6.2*2.4) + (2*6.2*3.6) + (2*2.4*3.6)), (int)get_area_3Dsquare(6.2, 2.4, 3.6));
}
