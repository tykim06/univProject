#include "hw5_1.h"
#include <stdio.h>
#include <string.h>

static const char MODEL_NAME = {"Sedan", "SUV", "VAN"};

void hw5_1_create(Car_t *p_car)
{
    memset(p_car, 0, 100*sizeof(Car_t));
}

void hw5_1_destroy(void)
{
}

int add_car(Car_t *p_car) {
    char name[20];
    int model, count;
    int i;

    scanf("%s", name);

    printf("0:Sedan/1:SUV/2:VAN\n");
    scanf("%d", &model);

    scanf("%d", &count);

    for(i=0;i<100;i++) {
        if(p_car[i].name[0] == 0) {
            strcpy(p_car[i].name, name);
            p_car[i].model = model;
            p_car[i].count = count;

            return i;
        }
    }
    return -1;
}

int find_car(Car_t *p_car) {
    char name[20];
    int i;

    scanf("%s", name);

    for(i=0;i<100;i++) {
        if(p_car[i].name[0] == 0) return -1;
        else if(strcmp(p_car[i].name, name) == 0) {
            printf("type:%s,number:%d\n", MODEL_NAME[p_car[i].model], p_car[i].count);
            return i;
        }
    }

    return -1;
}
