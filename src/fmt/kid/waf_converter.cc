// KID music/sound file
//
// Company:   KID
// Engine:    -
// Extension: .waf
// Archives:  LNK
//
// Known games:
// - Ever 17

#include "fmt/kid/waf_converter.h"

using namespace au;
using namespace au::fmt::kid;

static const bstr magic = "WAF\x00\x00\x00"_b;

bool WafConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> WafConverter::decode_internal(File &file) const
{
    std::unique_ptr<File> output_file(new File);

    file.io.skip(6);

    output_file->io.write("RIFF"_b);
    output_file->io.write("\x00\x00\x00\x00"_b);
    output_file->io.write("WAVE"_b);
    output_file->io.write("fmt\x20"_b);
    output_file->io.write_u32_le(50); //fmt header size
    output_file->io.write_u16_le(2); //compression type - some kind of ADPCM
    output_file->io.write_u16_le(file.io.read_u16_le()); //channels
    output_file->io.write_u32_le(file.io.read_u32_le()); //sample rate
    output_file->io.write_u32_le(file.io.read_u32_le()); //average bytes / sec?
    output_file->io.write_u16_le(file.io.read_u16_le()); //block align?
    output_file->io.write_u16_le(file.io.read_u16_le()); //bits per sample?

    output_file->io.write_u16_le(32); //additional header size?
    output_file->io.write(file.io.read(32)); //aditional header?

    output_file->io.write("data"_b);
    output_file->io.write(file.io.read_to_eof());

    output_file->io.seek(4);
    output_file->io.write_u32_le(file.io.size());

    output_file->name = file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::Registry::add<WafConverter>("kid/waf");
