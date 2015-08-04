// CPS image container
//
// Company:   KID
// Engine:    -
// Extension: .cps
// Archives:  LNK
//
// Known games:
// - Ever 17

#include "fmt/kid/cps_converter.h"
#include "fmt/kid/decompressor.h"
#include "io/buffered_io.h"

using namespace au;
using namespace au::fmt::kid;

static const std::string magic = "CPS\x00"_s;

static std::unique_ptr<io::IO> decrypt(
    io::IO &input_io, size_t size_compressed, size_t offset)
{
    std::unique_ptr<io::BufferedIO> output_io(new io::BufferedIO);
    output_io->reserve(input_io.size());

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
        output_io->write_u32_le(value);
        key = key * 0x41C64E6D + 0x9B06;
    }
    output_io->write_u32_le(0);

    output_io->seek(4);
    return std::unique_ptr<io::IO>(std::move(output_io));
}

bool CpsConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> CpsConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    size_t size_compressed = file.io.read_u32_le();
    auto version = file.io.read_u16_le();
    auto compression_type = file.io.read_u16_le();
    size_t size_original = file.io.read_u32_le();

    std::unique_ptr<io::IO> tmp_io(new io::BufferedIO);
    tmp_io->write_from_io(file.io, size_compressed - 16 - 4);

    auto offset = file.io.read_u32_le() - 0x7534682;
    if (offset)
        tmp_io = decrypt(*tmp_io, size_compressed, offset);

    if (compression_type & 1)
        tmp_io = decompress(*tmp_io, size_original);

    std::unique_ptr<File> output_file(new File);
    output_file->io.write_from_io(*tmp_io);
    output_file->name = file.name;
    output_file->change_extension("prt");
    return output_file;
}
