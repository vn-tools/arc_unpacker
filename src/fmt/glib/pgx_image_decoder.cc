#include "fmt/glib/pgx_image_decoder.h"
#include "fmt/glib/custom_lzss.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::glib;

static const bstr magic = "PGX\x00"_b;

bool PgxImageDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> PgxImageDecoder::decode_internal(File &file) const
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

    auto target = custom_lzss_decompress(source, target_size);

    pix::Grid pixels(width, height, target, pix::Format::BGRA8888);
    if (!transparent)
        for (auto y : util::range(height))
            for (auto x : util::range(width))
                pixels.at(x, y).a = 0xFF;

    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<PgxImageDecoder>("glib/pgx");
