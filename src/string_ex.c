#include <errno.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "assert_ex.h"
#include "logger.h"
#include "string_ex.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

char *strndup(const char *source, const size_t size)
{
    char *target = (char*)malloc(size + 1);
    assert_not_null(target);
    memcpy(target, source, size);
    target[size] = '\0';
    return target;
}

char *strdup(const char *source)
{
    return strndup(source, strlen(source));
}

void trim_right(char *target, const char *chars)
{
    char *end = target + strlen(target) - 1;
    while (end >= target && strchr(chars, *end) != NULL)
        end --;
    end[1] = '\0';
}

bool zlib_inflate(
    const char *input,
    size_t input_size,
    char **output,
    size_t *output_size)
{
    z_stream stream;
    assert_not_null(input);
    assert_not_null(output);

    size_t tmp;
    if (output_size == NULL)
        output_size = &tmp;

    if (input_size < SIZE_MAX / 10)
        *output_size = input_size;
    else
        *output_size = input_size;
    *output = (char*)malloc(*output_size + 1);
    assert_not_null(*output);

    stream.next_in = (unsigned char *)input;
    stream.avail_in = input_size;
    stream.next_out = (unsigned char *)*output;
    stream.avail_out = *output_size;
    stream.zalloc = (alloc_func)NULL;
    stream.zfree = (free_func)NULL;
    stream.opaque = (voidpf)NULL;
    stream.total_out = 0;

    assert_equali(Z_OK, inflateInit(&stream));

    while (1)
    {
        int result = inflate(&stream, Z_FINISH);
        switch (result)
        {
            case Z_STREAM_END:
                assert_equali(Z_OK, inflateEnd(&stream));
                *output = (char*)realloc(*output, stream.total_out + 1);
                assert_not_null(*output);
                *output_size = stream.total_out;
                (*output)[*output_size] = '\0';
                return true;

            case Z_BUF_ERROR:
            case Z_OK:
                if (*output_size < SIZE_MAX / 2)
                {
                    *output_size *= 2;
                }
                else if (*output_size == SIZE_MAX - 1)
                {
                    log_error("input too large");
                    free(*output);
                    *output = NULL;
                    *output_size = 0;
                    return false;
                }
                else
                {
                    *output_size = SIZE_MAX - 1;
                }
                *output = (char*)realloc(*output, *output_size + 1);
                assert_not_null(*output);
                stream.next_out = (unsigned char *)*output + stream.total_out;
                stream.avail_out = *output_size - stream.total_out;
                break;

            default:
                log_error("Inflate failed: %d", result);
                free(*output);
                *output = NULL;
                *output_size = 0;
                return false;
        }
    }

    log_error("???");
    return false;
}

bool convert_encoding(
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
