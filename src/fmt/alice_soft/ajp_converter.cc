// AJP JPEG image wrapper
//
// Company:   Alice Soft
// Engine:    -
// Extension: .ajp
// Archives:  AFA
//
// Known games:
// - Daiakuji

#include <iostream>
#include "fmt/alice_soft/ajp_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::alice_soft;

static const std::string magic = "AJP\x00"_s;
static const std::string key =
    "\x5d\x91\xAE\x87\x4A\x56\x41\xCD\x83\xEC\x4C\x92\xB5\xCB\x16\x34";

static std::unique_ptr<io::IO> decrypt(io::IO &input_io, size_t size)
{
    std::unique_ptr<io::BufferedIO> output_io(new io::BufferedIO);
    output_io->reserve(size);
    for (auto i : util::range(key.size()))
        if (output_io->tell() < size)
            output_io->write_u8(input_io.read_u8() ^ key[i]);
    output_io->write_from_io(input_io, size - output_io->tell());
    output_io->seek(0);
    return std::unique_ptr<io::IO>(std::move(output_io));
}

bool AjpConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> AjpConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    file.io.skip(4 * 2);
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto jpeg_offset = file.io.read_u32_le();
    auto jpeg_size = file.io.read_u32_le();
    auto mask_offset = file.io.read_u32_le();
    auto mask_size = file.io.read_u32_le();

    file.io.seek(jpeg_offset);
    auto jpeg_io = decrypt(file.io, jpeg_size);

    if (mask_size)
    {
        file.io.seek(mask_offset);
        auto mask_io = decrypt(file.io, mask_size);

        //TODO:
        //1. add PM converter
        //2. pass the mask through the PM converter
        //3. apply the mask to JPEG image
    }

    std::unique_ptr<File> output_file(new File);
    output_file->io.write_from_io(*jpeg_io);
    output_file->name = file.name;
    output_file->guess_extension();
    return output_file;
}
