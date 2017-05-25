#include <stdio.h>
#include "hw4_2.h"
#include <string.h>

int main(void) {
    char str[101];
    char find[16];
    char flag = '1';

    while(flag == '1') {
        fgets(str, sizeof(str), stdin);
        str[strlen(str)-1] = '\0';
        fgets(find, sizeof(find), stdin);
        find[strlen(find)-1] = '\0';

        printf("exist : %s\n", get_p_switched_case(str, find));
        printf("run-1 or stop-0 : ");
        flag = getchar();
        getchar();
    }
    return 0;
}
