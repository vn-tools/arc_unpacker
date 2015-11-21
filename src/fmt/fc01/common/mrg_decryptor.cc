#include "fmt/fc01/common/mrg_decryptor.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;
using namespace au::fmt::fc01::common;

static u16 get_mask(u16 d)
{
    d--;
    d >>= 8;
    u16 result = 0xFF;
    while (d)
    {
        d >>= 1;
        result = (result << 1) | 1;
    }
    return result;
}

struct MrgDecryptor::Priv final
{
    Priv(const bstr &input);
    Priv(const bstr &input, const size_t size_orig);

    io::MemoryStream input_stream;
    size_t size_orig;
};

MrgDecryptor::Priv::Priv(const bstr &input) : input_stream(input)
{
    // the size is encoded in the stream
    input_stream.seek(0x104);
    size_orig = input_stream.read_u32_le();
    input_stream.seek(0);
    size_orig ^= input_stream.read_u32_le();
    input_stream.seek(4);
}

MrgDecryptor::Priv::Priv(const bstr &input, const size_t size_orig)
    : input_stream(input), size_orig(size_orig)
{
    // the size is given from the outside
}

MrgDecryptor::MrgDecryptor(const bstr &input) : p(new Priv(input))
{
}

MrgDecryptor::MrgDecryptor(const bstr &input, const size_t size_orig)
    : p(new Priv(input, size_orig))
{
}

MrgDecryptor::~MrgDecryptor()
{
}

bstr MrgDecryptor::decrypt_without_key()
{
    return decrypt_with_key(0);
}

bstr MrgDecryptor::decrypt_with_key(const u8 initial_key)
{
    bstr output(p->size_orig);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    u16 arr1[0x101] {};
    u16 arr2[0x101] {};
    std::vector<u8> arr3;

    auto key = initial_key;
    for (auto i : util::range(0x100))
    {
        auto byte = p->input_stream.read_u8();
        if (initial_key)
        {
            byte = (((byte << 1) | (byte >> 7)) ^ key);
            key -= i;
        }

        arr2[i] = byte;
        arr1[i + 1] = arr2[i] + arr1[i];
        for (auto j : util::range(arr2[i]))
            arr3.push_back(i);
    }

    u16 quant = arr1[0x100];
    if (!quant)
        throw err::CorruptDataError("Unexpected data");
    auto mask = get_mask(quant);
    auto scale = 0x10000 / quant;
    auto a = p->input_stream.read_u32_be();
    auto b = 0;
    auto c = 0xFFFFFFFF;
    while (output_ptr < output_end)
    {
        c = ((c >> 8) * scale) >> 8;
        auto v = (a - b) / c;
        if (v > quant)
            throw err::CorruptDataError("Invalid quant");
        v = arr3.at(v);
        *output_ptr++ = v;
        b += arr1[v] * c;
        c *= arr2[v];
        while (!(((c + b) ^ b) & 0xFF000000))
        {
            a <<= 8;
            b <<= 8;
            c <<= 8;
            a |= p->input_stream.read_u8();
        }
        while (c <= mask)
        {
            c = (~b & mask) << 8;
            a = (a << 8) | p->input_stream.read_u8();
            b <<= 8;
        }
    }

    return output;
}
