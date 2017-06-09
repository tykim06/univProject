#include "hw4_3.h"
#include <string.h>

int digit[10];
int sum;
int digital;

void hw4_3_create(void)
{
    memset(digit, 0, 10);
    sum = 0;
    digital = 0;
}

void hw4_3_destroy(void)
{
}

int *count_digit(int N) {
	if(N/10 == 0) {
		digit[N]++;
		return digit;
	}

	digit[N%10]++;
	return count_digit(N/10);
}

int sum_digit(int N) {
	if(N/10 == 0) {
		sum += N;
		return sum;
	}

	sum += N%10;
	return sum_digit(N/10);
}

int add_digital(int N) {
	if(N/10 == 0) {
		digital += N;
		if(digital/10 != 0) {
			N = digital;
			digital = 0;
		} else {
            return digital;
        }
	}

	digital += N%10;
	return add_digital(N/10);
}
