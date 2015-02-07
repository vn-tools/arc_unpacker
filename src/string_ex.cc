#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iconv.h>
#include <stdexcept>
#include <zlib.h>
#include "logger.h"
#include "string_ex.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

std::string zlib_inflate(const std::string &input)
{
    std::string output;
    size_t written = 0;
    output.reserve(input.size() * ((input.size() < SIZE_MAX / 10) ? 3 : 1));

    z_stream stream;
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
    stream.avail_in = input.size();
    stream.zalloc = (alloc_func)nullptr;
    stream.zfree = (free_func)nullptr;
    stream.opaque = (voidpf)nullptr;
    stream.total_out = 0;

    if (inflateInit(&stream) != Z_OK)
        throw std::runtime_error("Failed to initialize zlib stream");

    int ret;
    do
    {
        char buffer[8192];
        stream.next_out = reinterpret_cast<Bytef*>(buffer);
        stream.avail_out = sizeof(buffer);
        ret = inflate(&stream, 0);
        if (output.size() < stream.total_out)
            output.append(buffer, stream.total_out - written);
        written = stream.total_out;
    }
    while (ret == Z_OK);

    inflateEnd(&stream);

    if (ret != Z_STREAM_END)
    {
        throw std::runtime_error(
            "Failed to inflate zlib stream (" + std::string(stream.msg) + ")");
    }

    return output;
}

bool convert_encoding(
    const char *input,
    size_t input_size,
    char **output,
    size_t *output_size,
    const char *from,
    const char *to)
{
    assert(input != nullptr);
    assert(output != nullptr);
    assert(from != nullptr);
    assert(to != nullptr);

    size_t tmp;
    if (output_size == nullptr)
        output_size = &tmp;

    iconv_t conv = iconv_open(to, from);

    *output = nullptr;
    *output_size = 0;

    char *output_old, *output_new;
    char *input_ptr = (char*) input;
    char *output_ptr = nullptr;
    size_t input_bytes_left = input_size;
    size_t output_bytes_left = *output_size;

    while (1)
    {
        output_old = *output;
        output_new = (char*)realloc(*output, *output_size);
        if (!output_new)
        {
            log_error("Encoding: Failed to allocate memory");
            free(*output);
            *output = nullptr;
            *output_size = 0;
            return false;
        }
        *output = output_new;

        if (output_old == nullptr)
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
                log_error("Encoding: Invalid byte sequence");
                free(*output);
                *output = nullptr;
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
