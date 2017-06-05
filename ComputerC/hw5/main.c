#include <stdio.h>
#include <stdlib.h>

int main(){

	/* Type or paste your code in this area */

    float *p_f_arr;
    int num, i;

    scanf("%d", &num);
    getchar();

    p_f_arr = malloc(num * sizeof(float));

    for(i=0;i<num;i++) {
        scanf("%f", &p_f_arr[i]);
    }

    for(i=num-1;i>=0;i--) {
        printf("%.1f\n", p_f_arr[i]);
    }

	return 0;
}
