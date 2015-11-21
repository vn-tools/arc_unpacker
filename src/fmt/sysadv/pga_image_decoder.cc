#include "fmt/sysadv/pga_image_decoder.h"

using namespace au;
using namespace au::fmt::sysadv;

static const bstr magic = "PGAPGAH\x0A"_b;

bool PgaImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> PgaImageDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());
    auto output_file = std::make_unique<File>();
    output_file->stream.write("\x89\x50\x4E\x47"_b);
    output_file->stream.write("\x0D\x0A\x1A\x0A"_b);
    output_file->stream.write("\x00\x00\x00\x0D"_b);
    output_file->stream.write("IHDR"_b);
    file.stream.seek(0xB);
    output_file->stream.write(file.stream.read_to_eof());
    output_file->name = file.name;
    output_file->change_extension("png");
    return output_file;
}

static auto dummy = fmt::register_fmt<PgaImageDecoder>("sysadv/pga");
