#include "hw3_1.h"
#include <math.h>

int get_max_power(int N) {
    int i = 0;

    while(pow(3, i++) < N);

    return i-2;
}

