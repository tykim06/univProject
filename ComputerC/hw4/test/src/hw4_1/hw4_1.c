#include "hw4_1.h"
#include <stdio.h>

void hw4_1_create(void)
{
}

void hw4_1_destroy(void)
{
}

void hw4_1_pointer(char arr[], char input) {
    char *ptr, *ptr2;

    if(input == '!') {
        ptr = arr;
        ptr2 = arr;
        while(1) {
            printf("%c ", *(ptr2));
            if(*ptr2 == 'Y') {
                printf("\n");
                break;
            }
            if((ptr2-ptr+1)%5 == 0 && ptr2 != arr) printf("\n");
            ptr2++;
        }
    } else if(input == '@') {
        ptr = arr;
        ptr2 = arr+1;
        while(1) {
            printf("%c ", *ptr);
            if(*ptr == 'Y') {
                printf("\n");
                break;
            }
            if(ptr-ptr2 == 19) {
                printf("\n");
                ptr = ptr2++;
            } else {
                ptr += 5;
            }
        }
    } else if(input == '#') {
        ptr = arr;
        ptr2 = arr + 24;
        while(1) {
            printf("%c ", *ptr);
            if(*ptr == 'M') {
                printf("\n");
                break;
            }
            if(((ptr-arr+1) + (arr+24-ptr2))%5 == 0) printf("\n");
            ptr++;
            printf("%c ", *(ptr2));
            if(((ptr-arr+1) + (arr+24-ptr2))%5 == 0) printf("\n");
            ptr2--;
        }
    }
}


