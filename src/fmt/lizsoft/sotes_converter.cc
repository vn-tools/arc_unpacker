// SOTES image
//
// Company:   Lizsoft
// Engine:    FOR-D System
// Extension: - (embedded in executables)
// Archives:  .dll
//
// Known games:
// - Fortune Summoners: Secret Of The Elemental Stone (SOTES)

#include <cstring>
#include "fmt/lizsoft/sotes_converter.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::lizsoft;

static size_t guess_image_dimension(
    const std::vector<u32> candidates,
    int main_delta,
    int max_delta_correction,
    size_t pixels_size)
{
    for (auto &base : candidates)
    {
        for (int delta = 0; delta <= max_delta_correction; delta++)
        {
            size_t possible_dimension = base + main_delta + delta;
            if (possible_dimension == 0)
                continue;
            if (possible_dimension > pixels_size)
                continue;
            if (pixels_size % possible_dimension == 0)
                return possible_dimension;
        }
    }
    throw std::runtime_error("Cannot figure out the image dimensions");
}

static void mirror(bstr &pixels, size_t stride)
{
    size_t height = pixels.size() / stride;
    bstr old_line;
    old_line.resize(stride);
    for (auto y : util::range(height / 2))
    {
        memcpy(
            old_line.get<u8>(),
            &pixels.get<u8>()[y * stride],
            stride);

        memcpy(
            &pixels.get<u8>()[y * stride],
            &pixels.get<u8>()[(height - 1 - y) * stride],
            stride);

        memcpy(
            &pixels.get<u8>()[(height - 1 - y) * stride],
            old_line.get<u8>(),
            stride);
    }
}

bool SotesConverter::is_recognized_internal(File &file) const
{
    u32 weird_data1[8];
    u32 weird_data2[14];
    file.io.read(weird_data1, 8 * 4);
    file.io.skip(256 * 4);
    file.io.read(weird_data2, 14 * 4);
    file.io.skip(8);
    if (file.io.read(2) != "BM"_b)
        return false;

    size_t pixel_data_offset = weird_data2[12] - weird_data2[10];
    if (pixel_data_offset >= file.io.size())
        return false;

    return true;
}

std::unique_ptr<File> SotesConverter::decode_internal(File &file) const
{
    if (file.io.size() < 1112)
        throw std::runtime_error("Not a SOTES image");

    u32 weird_data1[8];
    u32 palette[256];
    u32 weird_data2[14];
    file.io.read(weird_data1, 8 * 4);
    file.io.read(palette, 256 * 4);
    file.io.read(weird_data2, 14 * 4);

    size_t pixel_data_offset = weird_data2[12] - weird_data2[10];
    file.io.skip(pixel_data_offset);

    size_t raw_data_size = file.io.size() - file.io.tell();

    size_t width = guess_image_dimension(
        std::vector<u32>(&weird_data1[1], &weird_data1[5]),
        -static_cast<i32>(weird_data1[6]),
        3,
        raw_data_size);

    size_t height = guess_image_dimension(
        std::vector<u32>(&weird_data2[0], &weird_data2[5]),
        -static_cast<i32>(weird_data2[10]),
        0,
        raw_data_size);

    bool use_palette = width * height * 3 != raw_data_size;

    bstr pixels;
    pixels.resize(width * height * 3);

    bstr data = file.io.read(raw_data_size);
    if (use_palette)
    {
        u8 *pixels_ptr = pixels.get<u8>();
        for (auto i : util::range(raw_data_size))
        {
            if (pixels_ptr >= pixels.get<u8>() + pixels.size())
                throw std::runtime_error("Trying to write pixels beyond EOF");
            u32 rgba = palette[data.get<u8>(i)];
            *pixels_ptr++ = rgba;
            *pixels_ptr++ = rgba >> 8;
            *pixels_ptr++ = rgba >> 16;
        }
    }
    else
    {
        pixels = data;
    }

    mirror(pixels, 3 * width);

    auto image = util::Image::from_pixels(
        width, height, pixels, util::PixelFormat::BGR);
    return image->create_file(file.name);
}
