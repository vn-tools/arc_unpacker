#include "fmt/fc01/common/util.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

u8 common::rol8(u8 x, size_t n)
{
    n &= 7;
    return (x << n) | (x >> (8 - n));
}

bstr common::fix_stride(
    const bstr &input, size_t width, size_t height, size_t depth)
{
    auto output_stride = width * (depth >> 3);
    auto input_stride = (((width * (depth >> 3)) + 3) / 4) * 4;
    bstr output(height * output_stride);
    for (auto y : util::range(height))
    {
        auto output_ptr = &output[y * output_stride];
        auto input_ptr = &input[y * input_stride];
        for (auto x : util::range(width * (depth >> 3)))
            *output_ptr++ = *input_ptr++;
    }
    return output;
}
