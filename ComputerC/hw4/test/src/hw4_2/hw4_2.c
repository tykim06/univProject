#include "hw4_2.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void hw4_2_create(void)
{
}

void hw4_2_destroy(void)
{
}

char *get_p_switched_case(char *str, char *find) {
    if(str == NULL || find == NULL) return NULL;
    if(strlen(str) < strlen(find)) return NULL;
    if(strchr(find, ' ')) return NULL;

    const size_t str_length = strlen(str);
    const size_t find_length = strlen(find);
    char upper_str[str_length+1];
    char upper_find[find_length+1];
    strcpy(upper_str, str);
    strcpy(upper_find, find);

    int i, gap = 'a'-'A';
    for(i=0;i<str_length;i++) {
        if(upper_str[i]>='a') upper_str[i] -= gap;
    }
    for(i=0;i<find_length;i++) {
        if(upper_find[i]>='a') upper_find[i] -= gap;
    }

    char *p_find = strstr(upper_str, upper_find);
    if(p_find == NULL) return NULL;
    int find_idx = p_find-upper_str;

    char *p_switched_word = &str[find_idx];
    for(i=0;i<find_length;i++) {
        if(p_switched_word[i]>='a') p_switched_word[i] -= gap;
        else p_switched_word[i] += gap;
    }
    p_switched_word[find_length] = '\0';

    return p_switched_word;
}

