#include <string.h>
#include "string_ext.h"

void trim_right(char *target, char *chars)
{
    char *end = target + strlen(target) - 1;
    while (end >= target && strchr(chars, *end) != NULL)
        end --;
    end[1] = '\0';
}
