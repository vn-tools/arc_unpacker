#include "fmt/touhou/crypt.h"
#include <algorithm>
#include <memory>
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

bool DecryptorContext::operator ==(const DecryptorContext &other) const
{
    return key == other.key
        && step == other.step
        && block_size == other.block_size
        && limit == other.limit;
}

static void decrypt(
    io::IO &input_io,
    io::IO &output_io,
    const DecryptorContext &context)
{
    int left = input_io.size();
    size_t current_block_size = context.block_size;
    u8 key = context.key;

    size_t shift = left % current_block_size;
    if (shift >= (current_block_size >> 2))
        shift = 0;
    shift += (left & 1);
    left -= shift;

    while (left > 0 && output_io.tell() < context.limit)
    {
        if (left < static_cast<int>(current_block_size))
            current_block_size = left;
        auto input_block = input_io.read(current_block_size);
        auto *input_block_ptr = input_block.get<const char>();
        bstr output_block(current_block_size);
        for (auto j : util::range(2))
        {
            char *output_block_ptr = &output_block[current_block_size - j - 1];
            for (auto i : util::range((current_block_size - j + 1) >> 1))
            {
                *output_block_ptr = *input_block_ptr++ ^ key;
                output_block_ptr -= 2;
                key += context.step;
            }
        }

        output_io.write(output_block);
        left -= current_block_size;
    }

    left += shift;
    left = std::min(left, static_cast<int>(input_io.size() - input_io.tell()));
    if (left > 0)
        output_io.write_from_io(input_io, left);
    output_io.seek(0);
}

bstr au::fmt::touhou::decrypt(
    const bstr &input, const DecryptorContext &context)
{
    io::BufferedIO input_io(input);
    io::BufferedIO output_io;
    ::decrypt(input_io, output_io, context);
    return output_io.read_to_eof();
}
