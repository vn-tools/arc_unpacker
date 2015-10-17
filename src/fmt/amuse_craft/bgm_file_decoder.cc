#include "fmt/amuse_craft/bgm_file_decoder.h"

using namespace au;
using namespace au::fmt::amuse_craft;

static const bstr magic = "BGM\x20"_b;

bool BgmFileDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> BgmFileDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size());

    // If I had to guess, I'd say these contain loop information.
    // In any case, these don't seem too important.
    file.io.skip(8);

    auto output_file = std::make_unique<File>();
    output_file->io.write(file.io.read_to_eof());
    output_file->name = file.name;
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<BgmFileDecoder>("amuse-craft/bgm");
