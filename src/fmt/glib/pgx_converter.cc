// PGX image
//
// Company:   Rune
// Engine:    GLib
// Extension: -
// Archives:  GLib2
//
// Known games:
// - Musume Shimai
// - Watashi no Puni Puni
// - Pure My Imouto Milk Purin

#include "fmt/glib/gml_decoder.h"
#include "fmt/glib/pgx_converter.h"
#include "util/range.h"
#include "util/image.h"

using namespace au;
using namespace au::fmt::glib;

static const bstr magic = "PGX\x00"_b;

bool PgxConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> PgxConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    file.io.skip(4);
    size_t width = file.io.read_u32_le();
    size_t height = file.io.read_u32_le();
    bool transparent = file.io.read_u16_le();
    file.io.skip(2);
    size_t source_size = file.io.read_u32_le();
    size_t target_size = width * height * 4;

    file.io.seek(file.io.size() - source_size);
    auto source = file.io.read(source_size);

    auto target = GmlDecoder::decode(source, target_size);

    pix::Grid pixels(width, height, target, pix::Format::BGRA8888);
    if (!transparent)
        for (auto y : util::range(height))
            for (auto x : util::range(width))
                pixels.at(x, y).a = 0xFF;

    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<PgxConverter>("glib/pgx");
