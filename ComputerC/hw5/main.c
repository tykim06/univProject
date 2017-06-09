#include <stdio.h>
#include <string.h>

static const char *MODEL_NAME[] = {"Sedan", "SUV", "VAN"};

typedef struct Car_s {
    char name[20];
    int model;
    int count;
} Car_t;

int add_car(Car_t *p_car) {
    char name[20];
    int model, count;
    int i;

    scanf("%s", name);
    getchar();

    printf("0:Sedan/1:SUV/2:VAN\n");
    scanf("%d", &model);
    getchar();

    scanf("%d", &count);
    getchar();

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
    getchar();

    for(i=0;i<100;i++) {
        if(p_car[i].name[0] == 0) return -1;
        else if(strcmp(p_car[i].name, name) == 0) {
            printf("type:%s,number:%d\n", MODEL_NAME[p_car[i].model], p_car[i].count);
            return i;
        }
    }

    return -1;
}

int main(){

	/* Type or paste your code in this area */

    char num;
    Car_t car[100];
    memset(car, 0, 100*sizeof(Car_t));

    while(1) {
        printf("1:add car/2:search car/3.quit\n");
        num = getchar();
        getchar();

        switch(num) {
            case '1':
                add_car(car);
                break;
            case '2':
                find_car(car);
                break;
            case '3':
                return;
            default:
                printf("input error\n");
                break;
        }
    }
	return 0;
}
