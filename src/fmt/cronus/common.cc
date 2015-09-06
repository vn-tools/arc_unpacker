#include "fmt/cronus/common.h"
#include "util/range.h"

using namespace au;

u32 fmt::cronus::get_delta_key(const bstr &input)
{
    auto key = 0;
    for (auto c : input)
        key += c;
    return key;
}

void fmt::cronus::delta_decrypt(bstr &buffer, u32 initial_key)
{
    auto current_key = initial_key;
    auto key_delta = initial_key % 32;
    for (auto i : util::range(buffer.size()))
    {
        buffer[i] ^= current_key;
        current_key += key_delta;
    }
}
