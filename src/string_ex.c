#include <errno.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "logger.h"
#include "string_ex.h"

void trim_right(char *target, const char *chars)
{
    char *end = target + strlen(target) - 1;
    while (end >= target && strchr(chars, *end) != NULL)
        end --;
    end[1] = '\0';
}

bool convert(
    const char *input,
    size_t input_size,
    char **output,
    size_t *output_size,
    const char *from,
    const char *to)
{
    assert_not_null(input);
    assert_not_null(output);
    assert_not_null(from);
    assert_not_null(to);

    size_t tmp;
    if (output_size == NULL)
        output_size = &tmp;

    iconv_t conv = iconv_open(to, from);

    *output = NULL;
    *output_size = 0;

    char *output_old, *output_new;
    char *input_ptr = (char*) input;
    char *output_ptr = NULL;
    size_t input_bytes_left = input_size;
    size_t output_bytes_left = *output_size;

    while (1)
    {
        output_old = *output;
        output_new = (char*)realloc(*output, *output_size);
        if (!output_new)
        {
            log_error("Failed to allocate memory");
            free(*output);
            *output = NULL;
            *output_size = 0;
            return false;
        }
        *output = output_new;

        if (output_old == NULL)
            output_ptr = *output;
        else
            output_ptr += *output - output_old;

        int result = iconv(
            conv,
            &input_ptr,
            &input_bytes_left,
            &output_ptr,
            &output_bytes_left);

        if (result != -1)
            break;

        switch (errno)
        {
            case EINVAL:
            case EILSEQ:
                log_error("Invalid byte sequence");
                free(*output);
                *output = NULL;
                *output_size = 0;
                return false;

            case E2BIG:
                *output_size += 1;
                output_bytes_left += 1;
                continue;
        }
    }

    *output = (char*)realloc(*output, (*output_size) + 1);
    (*output)[(*output_size)] = '\0';
    iconv_close(conv);
    return true;
}
