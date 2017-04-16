#include "square.h"

void square_create(void)
{
}

void square_destroy(void)
{
}

float get_area_3Dsquare(float x, float y, float z)
{
    if(x < 0 || y < 0 || z < 0) {
        return -1;
    }

    return (2*x*y) + (2*x*z) + (2*y*z);
}

