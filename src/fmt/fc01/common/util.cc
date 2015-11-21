#include "fmt/fc01/common/util.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

u8 common::rol8(const u8 x, size_t n)
{
    n &= 7;
    return (x << n) | (x >> (8 - n));
}

bstr common::fix_stride(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t depth)
{
    const auto output_stride = width * (depth >> 3);
    const auto input_stride = ((output_stride + 3) / 4) * 4;
    bstr output(height * output_stride);
    for (const auto y : util::range(height))
    {
        auto output_ptr = &output[y * output_stride];
        const auto *input_ptr = &input[y * input_stride];
        for (const auto x : util::range(width * (depth >> 3)))
            *output_ptr++ = *input_ptr++;
    }
    return output;
}
