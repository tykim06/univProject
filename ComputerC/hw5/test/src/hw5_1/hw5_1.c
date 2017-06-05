#include "hw5_1.h"
#include <stdio.h>
#include <string.h>

static const char *MODEL_NAME[] = {"Sedan", "SUV", "VAN"};

void hw5_1_create(Car_t *p_car)
{
    memset(p_car, 0, 100*sizeof(Car_t));
}

void hw5_1_destroy(void)
{
}

int add_car(Car_t *p_cars, Car_t car) {
    int i;

    for(i=0;i<100;i++) {
        if(p_cars[i].name[0] == 0) {
            strcpy(p_cars[i].name, car.name);
            p_cars[i].model = car.model;
            p_cars[i].count = car.count;

            return i;
        }
    }
    return -1;
}

int find_car(Car_t *p_cars, char *p_name) {
    int i;

    for(i=0;i<100;i++) {
        if(p_cars[i].name[0] == 0) return -1;
        else if(strcmp(p_cars[i].name, p_name) == 0) {
            return i;
        }
    }

    return -1;
}
