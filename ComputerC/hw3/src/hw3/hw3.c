#include "hw3.h"
#include <math.h>

int get_max_power(int N) {
    int i = 0;

    while(pow(3, i++) < N);

    return i-2;
}

float calculator(int a, char c, int b) {
    if(c == '+') {
        return (float)(a + b);
    } else if(c == '-') {
        return (float)(a - b);
    } else if(c == '*') {
        return (float)(a * b);
    } else {
        return (float)a / b;
    }
    return 3.0;
}

void get_reversed_string(char *base, char *reversed, int length) {
    int i = length;
    while(i--) {
        reversed[i] = base[length - i - 1];
    }
}
