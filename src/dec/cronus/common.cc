#include "dec/cronus/common.h"
#include "algo/range.h"

using namespace au;

u32 dec::cronus::get_delta_key(const bstr &input)
{
    auto key = 0;
    for (const auto c : input)
        key += c;
    return key;
}

void dec::cronus::delta_decrypt(bstr &input, u32 initial_key)
{
    auto current_key = initial_key;
    const auto key_delta = initial_key % 32;
    for (const auto i : algo::range(input.size()))
    {
        input[i] ^= current_key;
        current_key += key_delta;
    }
}
