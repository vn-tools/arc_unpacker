#include "fmt/shiina_rio/warc/decompress.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::shiina_rio;
using namespace au::fmt::shiina_rio::warc;

namespace
{
    class CustomBitReader final
    {
    public:
        CustomBitReader(const bstr &input);
        u32 get(int bits);

    private:
        void fetch();

        io::BufferedIO input_io;
        int available_bits;
        u32 value;
    };
}

CustomBitReader::CustomBitReader(const bstr &input)
    : input_io(input), available_bits(0), value(0)
{
}

void CustomBitReader::fetch()
{
    if (input_io.size() - input_io.tell() >= 4)
    {
        value = input_io.read_u32_le();
        return;
    }
    while (!input_io.eof())
    {
        value <<= 8;
        value |= input_io.read_u8();
    }
}

u32 CustomBitReader::get(int requested_bits)
{
    u32 ret = 0;
    if (available_bits < requested_bits)
    {
        do
        {
            requested_bits -= available_bits;
            const u32 mask = (1ull << available_bits) - 1;
            ret |= (value & mask) << requested_bits;
            fetch();
            available_bits = 32;
        }
        while (requested_bits > 32);
    }
    available_bits -= requested_bits;
    const u32 mask = (1ull << requested_bits) - 1;
    return ret | ((value >> available_bits) & mask);
}

static int init_huffman(
    CustomBitReader &bit_reader, u16 nodes[2][512], int &size)
{
    if (!bit_reader.get(1))
        return bit_reader.get(8);
    const auto pos = size;
    if (pos > 511)
        return -1;
    size++;
    nodes[0][pos] = init_huffman(bit_reader, nodes, size);
    nodes[1][pos] = init_huffman(bit_reader, nodes, size);
    return pos;
}

static bstr decode_huffman(const bstr &input, const size_t size_orig)
{
    bstr output(size_orig);
    auto output_ptr = output.get<u8>();
    const auto output_end = output.end<const u8>();
    CustomBitReader bit_reader(input);
    u16 nodes[2][512];
    auto size = 256;
    auto root = init_huffman(bit_reader, nodes, size);
    while (output_ptr < output_end)
    {
        auto byte = root;
        while (byte >= 256 && byte <= 511)
            byte = nodes[bit_reader.get(1)][byte];
        *output_ptr++ = byte;
    }
    return output;
}

bstr warc::decompress_yh1(
    const bstr &input, const size_t size_orig, const bool encrypted)
{
    bstr transient(input);
    if (encrypted)
    {
        const u32 key32 = 0x6393528E;
        const u16 key16 = 0x4B4D;
        for (auto i : util::range(transient.size() / 4))
            transient.get<u32>()[i] ^= key32 ^ key16;
    }
    return decode_huffman(transient, size_orig);
}

bstr warc::decompress_ypk(
    const bstr &input, const size_t size_orig, const bool encrypted)
{
    bstr transient(input);
    if (encrypted)
    {
        const u16 key16 = 0x4B4D;
        const u32 key32 = (key16 | (key16 << 16)) ^ 0xFFFFFFFF;
        size_t i = 0;
        while (i < transient.size() / 4)
            transient.get<u32>()[i++] ^= key32;
        i *= 4;
        while (i < transient.size())
            transient[i++] ^= key32;
    }
    return util::pack::zlib_inflate(transient);
}

bstr warc::decompress_ylz(
    const bstr &input, const size_t size_orig, const bool encrypted)
{
    throw err::NotSupportedError("YLZ decompression not implemented");
}
