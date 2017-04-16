#include "string.h"

void string_create(void)
{
}

void string_destroy(void)
{
}

int char_to_index(char ch)
{
    if(ch > 'z' || ch < 'a') {
        return -1;
    }

    return ch - 'a' + 1;
}

