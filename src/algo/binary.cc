#include "algo/binary.h"
#include "algo/range.h"
#include "err.h"

using namespace au;

bstr algo::xor(const bstr &input, const u8 key)
{
    bstr output(input);
    for (const auto i : algo::range(input.size()))
        output[i] ^= key;
    return output;
}

bstr algo::xor(const bstr &input, const bstr &key)
{
    if (!key.size())
        throw err::BadDataSizeError();
    bstr output(input);
    for (const auto i : algo::range(input.size()))
        output[i] ^= key[i % key.size()];
    return output;
}
