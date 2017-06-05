#include <stdio.h>
#include "hw5_1.h"

int main(void) {
    int num;
    Car_t car[100];

    while(1) {
        printf("1:add car/2:search car/3.quit\n");
        scanf("%d", &num);

        switch(num) {
            case 1:
                add_car(car);
                break;
            case 2:
                find_car(car);
                break;
            case 3:
                return;
            default:
                printf("input error\n");
                break;
        }
    }
    return 0;
}
