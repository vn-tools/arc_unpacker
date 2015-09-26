#include "fmt/kid/cps_file_decoder.h"
#include "fmt/kid/decompressor.h"
#include "io/buffered_io.h"

using namespace au;
using namespace au::fmt::kid;

static const bstr magic = "CPS\x00"_b;

static bstr decrypt(const bstr &input, size_t size_compressed, size_t offset)
{
    io::BufferedIO input_io(input);
    io::BufferedIO output_io;
    output_io.reserve(input_io.size());

    auto real_offset = offset - 16;
    input_io.seek(real_offset);
    u32 key = input_io.read_u32_le() + offset + 0x3786425;

    input_io.seek(0);
    while (!input_io.eof())
    {
        bool use_key = input_io.tell() != real_offset;
        auto value = input_io.read_u32_le();
        if (use_key)
        {
            value -= size_compressed;
            value -= key;
        }
        output_io.write_u32_le(value);
        key = key * 0x41C64E6D + 0x9B06;
    }
    output_io.write_u32_le(0);

    output_io.seek(4);
    return output_io.read_to_eof();
}

bool CpsFileDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> CpsFileDecoder::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    size_t size_compressed = file.io.read_u32_le();
    auto version = file.io.read_u16_le();
    auto compression_type = file.io.read_u16_le();
    size_t size_original = file.io.read_u32_le();

    auto data = file.io.read(size_compressed - 16 - 4);

    auto offset = file.io.read_u32_le() - 0x7534682;
    if (offset)
        data = decrypt(data, size_compressed, offset);

    if (compression_type & 1)
        data = decompress(data, size_original);

    std::unique_ptr<File> output_file(new File);
    output_file->io.write(data);
    output_file->name = file.name;
    output_file->change_extension("prt");
    return output_file;
}

static auto dummy = fmt::Registry::add<CpsFileDecoder>("kid/cps");
