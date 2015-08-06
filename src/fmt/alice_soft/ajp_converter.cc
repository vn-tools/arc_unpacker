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
#include "util/range.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::alice_soft;

static const std::string magic = "AJP\x00"_s;

bool AjpConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> AjpConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto version = file.io.read_u32_le();
    auto header1_size = file.io.read_u32_le();
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto header2_size = file.io.read_u32_le();
    auto jpeg_data_size = file.io.read_u32_le();
    auto alpha_location = file.io.read_u32_le();
    auto additional_data_size = file.io.read_u32_le();
    file.io.skip(16 + 4 + 16);

    std::unique_ptr<File> output_file(new File);
    output_file->io.write("\xFF\xD8\xFF\xE0\x00\x10JFIF\x00\x01\x02\x00"_s);
    output_file->io.write_u16_le(file.io.read_u16_le());
    file.io.skip(-2);
    output_file->io.write_from_io(file.io, jpeg_data_size);

    output_file->name = file.name;
    output_file->guess_extension();
    return output_file;

    //apparently, it is followed by some kind of alpha channel
}
