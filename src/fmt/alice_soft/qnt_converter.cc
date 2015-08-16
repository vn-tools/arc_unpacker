// QNT image
//
// Company:   Alice Soft
// Engine:    -
// Extension: .qnt
// Archives:  AFA
//
// Known games:
// - Daiakuji

#include "fmt/alice_soft/qnt_converter.h"
#include "util/image.h"
#include "util/format.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic = "QNT\x00"_b;

namespace
{
    enum Version
    {
        Version0,
        Version1,
        Version2,
    };
}

static void deinterleave(pix::Grid &pixels, const bstr &input)
{
    size_t expected_size = 3;
    expected_size *= ((pixels.width() + 1) / 2) * 2;
    expected_size *= ((pixels.height() + 1) / 2) * 2;
    if (input.size() != expected_size)
        throw std::runtime_error("Unexpected color data size");

    const u8 *input_ptr = input.get<const u8>();
    size_t x, y;
    for (size_t i : util::range(3))
    {
        for (y = 0; y < pixels.height() - 1; y += 2)
        {
            for (x = 0; x < pixels.width() - 1; x += 2)
            {
                pixels.at(x, y)[i] = input_ptr[0];
                pixels.at(x, y + 1)[i] = input_ptr[1];
                pixels.at(x + 1, y)[i] = input_ptr[2];
                pixels.at(x + 1, y + 1)[i] = input_ptr[3];
                input_ptr += 4;
            }
            if (x != pixels.width())
            {
                pixels.at(x, y)[i] = input_ptr[0];
                pixels.at(x, y + 1)[i] = input_ptr[1];
                input_ptr += 4;
            }
        }
        if (y != pixels.height())
        {
            for (x = 0; x < pixels.width() - 1; x += 2)
            {
                pixels.at(x, y)[i] = input_ptr[0];
                pixels.at(x + 1, y)[i] = input_ptr[2];
                input_ptr += 4;
            }
            if (x != pixels.width())
            {
                pixels.at(x, y)[i] = input_ptr[0];
                input_ptr += 4;
            }
        }
    }
}

static void apply_differences(pix::Grid &pixels)
{
    for (size_t x : util::range(1, pixels.width()))
        for (size_t c : util::range(3))
            pixels.at(x, 0)[c] = pixels.at(x - 1, 0)[c] - pixels.at(x, 0)[c];

    for (size_t y : util::range(1, pixels.height()))
        for (size_t c : util::range(3))
            pixels.at(0, y)[c] = pixels.at(0, y - 1)[c] - pixels.at(0, y)[c];

    for (size_t y : util::range(1, pixels.height()))
    {
        for (size_t x : util::range(1, pixels.width()))
        {
            for (size_t c : util::range(3))
            {
                u8 ax = pixels.at(x - 1, y)[c];
                u8 ay = pixels.at(x, y - 1)[c];
                pixels.at(x, y)[c] = (ax + ay) / 2 - pixels.at(x, y)[c];
            }
        }
    }
}

static void apply_alpha(pix::Grid &pixels, const bstr &input)
{
    if (!input.size())
    {
        for (auto y : util::range(pixels.height()))
        for (auto x : util::range(pixels.width()))
            pixels.at(x, y).a = 0xFF;
        return;
    }

    size_t expected_size = pixels.width() * pixels.height();
    expected_size += (pixels.width() & 1) * pixels.height();
    expected_size += (pixels.height() & 1) * pixels.width();
    expected_size += (pixels.height() & 1) * (pixels.width() & 1);
    if (input.size() != expected_size)
        throw std::runtime_error("Unexpected alpha data size");

    const u8 *input_ptr = input.get<const u8>();
    if (pixels.width() > 1)
    {
        pixels.at(0, 0).a = *input_ptr++;
        for (size_t x : util::range(1, pixels.width()))
            pixels.at(x, 0).a = pixels.at(x - 1, 0).a - *input_ptr++;
        input_ptr += pixels.width() & 1;
    }
    else
        input_ptr++;

    if (pixels.height() > 1)
    {
        for (size_t y : util::range(1, pixels.height()))
        {
            pixels.at(0, y).a = pixels.at(0, y - 1).a - *input_ptr++;
            for (size_t x : util::range(1, pixels.width()))
            {
                u8 ax = pixels.at(x - 1, y).a;
                u8 ay = pixels.at(x, y - 1).a;
                pixels.at(x, y).a = (ax + ay) / 2 - *input_ptr++;
            }
            input_ptr += pixels.width() & 1;
        }
    }
}

bool QntConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> QntConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    Version version = static_cast<Version>(file.io.read_u32_le());

    if (version != Version2)
    {
        throw std::runtime_error(
            util::format("Unsupported version: %d", version));
    }

    file.io.skip(4 * 3);
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto depth = file.io.read_u32_le();
    file.io.skip(4);
    auto pixel_size = file.io.read_u32_le();
    auto alpha_size = file.io.read_u32_le();
    file.io.skip(24);

    bstr color_data = util::pack::zlib_inflate(file.io.read(pixel_size));
    bstr alpha_data;
    if (alpha_size)
        alpha_data = util::pack::zlib_inflate(file.io.read(alpha_size));

    pix::Grid pixels(width, height);
    deinterleave(pixels, color_data);
    apply_differences(pixels);
    apply_alpha(pixels, alpha_data);

    return util::Image::from_pixels(pixels)->create_file(file.name);
}
