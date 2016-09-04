#include "dec/kaguya/common/rle.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::kaguya;

bstr common::decompress_rle(
    const bstr &input, const size_t size_orig, const size_t bands)
{
    bstr output(size_orig);
    io::MemoryByteStream input_stream(input);

    for (const auto i : algo::range(bands))
    {
        auto output_ptr = algo::make_ptr(output) + i;
        const auto init_b = input_stream.read<u8>();
        auto last_b = init_b;
        if (!output_ptr.left())
            throw err::EofError();
        *output_ptr = init_b;
        output_ptr += bands;

        while (output_ptr.left())
        {
            auto b = input_stream.read<u8>();
            *output_ptr = b;
            output_ptr += bands;
            if (last_b == b)
            {
                u16 repetitions = input_stream.read<u8>();
                if (repetitions >= 0x80)
                {
                    repetitions &= 0x7F;
                    repetitions <<= 8;
                    repetitions |= input_stream.read<u8>();
                    repetitions += 0x80;
                }
                while (repetitions-- && output_ptr.left())
                {
                    *output_ptr = b;
                    output_ptr += bands;
                }

                if (output_ptr.left())
                {
                    b = input_stream.read<u8>();
                    *output_ptr = b;
                    output_ptr += bands;
                }
            }
            last_b = b;
        }
    }

    return output;
}
