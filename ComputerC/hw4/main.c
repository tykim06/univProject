#include <stdio.h>
#include "hw4_1.h"

int main(void) {
    char arr[25] = { '0' };
    int i = 0;
    char A = 'A';
    char input;

    for(i=0;i<25;i++)
        arr[i] = A++;

    scanf("%c", &input);

    hw4_1_pointer(arr, input);

    return 0;
}
