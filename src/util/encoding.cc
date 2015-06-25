#include <cerrno>
#include <memory>
#include <stdexcept>
#include <iconv.h>
#include "util/encoding.h"

std::string convert_encoding(
    const std::string &input, const std::string &from, const std::string &to)
{
    iconv_t conv = iconv_open(to.c_str(), from.c_str());
    if (conv == nullptr)
        throw std::runtime_error("Failed to initialize iconv");

    std::string output;
    output.reserve(input.size() * 2);

    char *input_ptr = const_cast<char*>(input.data());
    size_t input_bytes_left = input.size();
    const size_t buffer_size = 32;
    std::unique_ptr<char[]> buffer(new char[buffer_size]);

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
