#include <cerrno>
#include <iconv.h>
#include <memory>
#include <stdexcept>
#include <zlib.h>
#include "logger.h"
#include "string_ex.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

std::string stoi(int i)
{
    if (i == 0)
        return "0";
    if (i < 0)
        return "-" + stoi(- i);

    std::string x;
    while (i)
    {
        char c = (i % 10) + '0';
        x = c + x;
        i /= 10;
    }
    return x;
}

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
        const size_t buffer_size = 8192;
        std::unique_ptr<char> output_buffer(new char[buffer_size]);
        stream.next_out = reinterpret_cast<Bytef*>(output_buffer.get());
        stream.avail_out = buffer_size;
        ret = inflate(&stream, 0);
        if (output.size() < stream.total_out)
            output.append(output_buffer.get(), stream.total_out - written);
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

std::string convert_encoding(
    const std::string &input,
    const std::string &from,
    const std::string &to)
{
    iconv_t conv = iconv_open(to.c_str(), from.c_str());
    if (conv == nullptr)
        throw std::runtime_error("Failed to initialize iconv");

    std::string output;
    output.reserve(input.size() * 2);

    char *input_ptr = const_cast<char*>(input.data());
    size_t input_bytes_left = input.size();
    const size_t buffer_size = 32;
    std::unique_ptr<char> buffer(new char[buffer_size]);

    while (true)
    {
        char *output_buffer = buffer.get();
        size_t output_bytes_left = buffer_size;
        int ret = iconv(
            conv,
            &input_ptr,
            &input_bytes_left,
            &output_buffer,
            &output_bytes_left);

        output.append(buffer.get(), buffer_size - output_bytes_left);

        if (ret != -1 && input_bytes_left == 0)
            break;

        switch (errno)
        {
            case EINVAL:
            case EILSEQ:
                throw std::runtime_error("Invalid byte sequence");

            case E2BIG:
                //repeat the iteration unless we got nothing at all
                if (output_bytes_left != buffer_size)
                    continue;
                throw std::runtime_error("Code point too large to decode (?)");
        }
    }

    return output;
}
