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

    // TODO: this looks light and pretty, think about merging it with Image
    struct PixelLocator
    {
        size_t x, y, channel;
    };

    struct PixelArray
    {
        PixelArray(size_t width, size_t height);
        inline u8 &operator[](const PixelLocator &loc);
        std::unique_ptr<util::Image> to_image() const;

        size_t width, height;
        bstr pixels;
    };
}

static void deinterleave(PixelArray &pix, const bstr &input)
{
    size_t expected_size = 3;
    expected_size *= ((pix.width + 1) / 2) * 2;
    expected_size *= ((pix.height + 1) / 2) * 2;
    if (input.size() != expected_size)
        throw std::runtime_error("Unexpected color data size");

    const u8 *input_ptr = input.get<const u8>();
    size_t x, y;
    for (size_t i : util::range(2, -1, -1))
    {
        for (y = 0; y < pix.height - 1; y += 2)
        {
            for (x = 0; x < pix.width - 1; x += 2)
            {
                pix[{x, y, i}] = input_ptr[0];
                pix[{x, y + 1, i}] = input_ptr[1];
                pix[{x + 1, y, i}] = input_ptr[2];
                pix[{x + 1, y + 1, i}] = input_ptr[3];
                input_ptr += 4;
            }
            if (x != pix.width)
            {
                pix[{x, y, i}] = input_ptr[0];
                pix[{x, y + 1, i}] = input_ptr[1];
                input_ptr += 4;
            }
        }
        if (y != pix.height)
        {
            for (x = 0; x < pix.width - 1; x += 2)
            {
                pix[{x, y, i}] = input_ptr[0];
                pix[{x + 1, y, i}] = input_ptr[2];
                input_ptr += 4;
            }
            if (x != pix.width)
            {
                pix[{x, y, i}] = input_ptr[0];
                input_ptr += 4;
            }
        }
    }
}

static void apply_differences(PixelArray &pix)
{
    for (size_t x : util::range(1, pix.width))
        for (size_t c : util::range(3))
            pix[{x, 0, c}] = pix[{x - 1, 0, c}] - pix[{x, 0, c}];

    for (size_t y : util::range(1, pix.height))
        for (size_t c : util::range(3))
            pix[{0, y, c}] = pix[{0, y - 1, c}] - pix[{0, y, c}];

    for (size_t y : util::range(1, pix.height))
    {
        for (size_t x : util::range(1, pix.width))
        {
            for (size_t c : util::range(3))
            {
                u8 ax = pix[{x - 1, y, c}];
                u8 ay = pix[{x, y - 1, c}];
                pix[{x, y, c}] = (ax + ay) / 2 - pix[{x, y, c}];
            }
        }
    }
}

static void apply_alpha(PixelArray &pix, const bstr &input)
{
    if (!input.size())
        return;

    size_t expected_size = pix.width * pix.height;
    expected_size += (pix.width & 1) * pix.height;
    expected_size += (pix.height & 1) * pix.width;
    expected_size += (pix.height & 1) * (pix.width & 1);
    if (input.size() != expected_size)
        throw std::runtime_error("Unexpected alpha data size");

    const u8 *input_ptr = input.get<const u8>();
    if (pix.width > 1)
    {
        pix[{0, 0, 3}] = *input_ptr++;
        for (size_t x : util::range(1, pix.width))
            pix[{x, 0, 3}] = pix[{x - 1, 0, 3}] - *input_ptr++;
        input_ptr += pix.width & 1;
    }
    else
        input_ptr++;

    if (pix.height > 1)
    {
        for (size_t y : util::range(1, pix.height))
        {
            pix[{0, y, 3}] = pix[{0, y - 1, 3}] - *input_ptr++;
            for (size_t x : util::range(1, pix.width))
            {
                u8 ax = pix[{x - 1, y, 3}];
                u8 ay = pix[{x, y - 1, 3}];
                pix[{x, y, 3}] = (ax + ay) / 2 - *input_ptr++;
            }
            input_ptr += pix.width & 1;
        }
    }
}

PixelArray::PixelArray(size_t width, size_t height)
    : width(width), height(height)
{
    pixels.resize(width * height * 4);
    memset(pixels.get<u8>(), 0xFF, pixels.size());
}

u8 &PixelArray::operator[](const PixelLocator &loc)
{
    return pixels.get<u8>((loc.x + loc.y * width) * 4 + loc.channel);
}

std::unique_ptr<util::Image> PixelArray::to_image() const
{
    return util::Image::from_pixels(
        width, height, pixels, util::PixelFormat::RGBA);
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

    PixelArray pix(width, height);
    deinterleave(pix, color_data);
    apply_differences(pix);
    apply_alpha(pix, alpha_data);

    auto image = pix.to_image();
    return image->create_file(file.name);
}
