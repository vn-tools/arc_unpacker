#include "fmt/kid/decompressor.h"
#include "util/range.h"

using namespace au;

bstr au::fmt::kid::decompress(const bstr &input, size_t size_original)
{
    bstr output;
    output.resize(size_original);

    u8 *output_ptr = output.get<u8>();
    const u8 *output_guardian = output_ptr + output.size();

    const u8 *input_ptr = input.get<u8>();

    while (output_ptr < output_guardian)
    {
        u8 byte = *input_ptr++;
        if (byte & 0x80)
        {
            if (byte & 0x40)
            {
                int repetitions = (byte & 0x1F) + 2;
                if (byte & 0x20)
                    repetitions += *input_ptr++ << 5;
                for (auto i : util::range(repetitions))
                    *output_ptr++ = *input_ptr;
                input_ptr++;
            }
            else
            {
                int length = ((byte >> 2) & 0xF) + 2;
                int look_behind = ((byte & 3) << 8) + *input_ptr++ + 1;
                if (look_behind < 0)
                    look_behind = 0;
                for (auto i : util::range(length))
                {
                    u8 tmp = output_ptr[-look_behind];
                    *output_ptr++ = tmp;
                }
            }
        }
        else
        {
            if (byte & 0x40)
            {
                int repetitions = *input_ptr++ + 1;
                int length = (byte & 0x3F) + 2;
                for (auto i : util::range(repetitions))
                    for (auto i : util::range(length))
                        *output_ptr++ += input_ptr[i];
                input_ptr += length;
            }
            else
            {
                int length = (byte & 0x1F) + 1;
                if (byte & 0x20)
                    length += *input_ptr++ << 5;
                for (auto i : util::range(length))
                    *output_ptr++ = *input_ptr++;
            }
        }
    }

    return output;
}
