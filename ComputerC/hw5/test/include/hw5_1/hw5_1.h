#ifndef D_hw5_1_H
#define D_hw5_1_H

/**********************************************************
 *
 * hw5_1 is responsible for ...
 *
 **********************************************************/
typedef struct Car_s {
    char name[20];
    int model;
    int count;
} Car_t;

void hw5_1_create(Car_t *p_car);
void hw5_1_destroy(void);
int add_car(Car_t *p_car);
int find_car(Car_t *p_car);

#endif  /* D_Fakehw5_1_H */
