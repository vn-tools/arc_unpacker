#include <memory>
#include "formats/touhou/crypt.h"
using namespace Formats::Touhou;

namespace
{
    inline int min(int a, int b) { return a < b ? a : b; }
}

bool DecryptorContext::operator ==(const DecryptorContext &other) const
{
    return key == other.key
        && step == other.step
        && block_size == other.block_size
        && limit == other.limit;
}

void Formats::Touhou::decrypt(
    IO &input, size_t size, IO &output, const DecryptorContext &context)
{
    int left = min(size, input.size() - input.tell());
    size_t current_block_size = context.block_size;
    uint8_t key = context.key;
    std::unique_ptr<char[]> input_block(new char[context.block_size]);
    std::unique_ptr<char[]> output_block(new char[context.block_size]);

    size_t shift = left % current_block_size;
    if (shift >= (current_block_size >> 2))
        shift = 0;
    shift += (left & 1);
    left -= shift;

    while (left > 0 && output.tell() < context.limit)
    {
        if (left < static_cast<int>(current_block_size))
            current_block_size = left;
        input.read(input_block.get(), current_block_size);

        const char *input_block_ptr = input_block.get();
        for (int j = 0; j < 2; j ++)
        {
            char *output_block_ptr
                = &output_block[current_block_size - j - 1];
            for (size_t i = 0; i < (current_block_size - j + 1) >> 1; i ++)
            {
                *output_block_ptr = *input_block_ptr ^ key;
                output_block_ptr -= 2;
                input_block_ptr ++;
                key += context.step;
            }
        }

        output.write(output_block.get(), current_block_size);
        left -= current_block_size;
    }

    left += shift;
    left = min(left, input.size() - input.tell());
    if (left > 0)
        output.write_from_io(input,left);

    output.seek(0);
}
