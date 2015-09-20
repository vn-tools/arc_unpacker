#include "fmt/lizsoft/sotes_converter.h"
#include "err.h"
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
    throw err::CorruptDataError("Cannot figure out the image dimensions");
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
        throw err::CorruptDataError("Not a SOTES image");

    u32 weird_data1[8];
    file.io.read(weird_data1, 8 * 4);

    pix::Palette palette(256, file.io, pix::Format::BGRA8888);

    u32 weird_data2[14];
    file.io.read(weird_data2, 14 * 4);

    size_t pixel_data_offset = weird_data2[12] - weird_data2[10];
    file.io.skip(pixel_data_offset);

    size_t raw_data_size = file.io.size() - file.io.tell();

    auto width = guess_image_dimension(
        std::vector<u32>(&weird_data1[1], &weird_data1[5]),
        -static_cast<s32>(weird_data1[6]),
        3,
        raw_data_size);

    size_t height = guess_image_dimension(
        std::vector<u32>(&weird_data2[0], &weird_data2[5]),
        -static_cast<s32>(weird_data2[10]),
        0,
        raw_data_size);

    bool use_palette = width * height * 3 != raw_data_size;

    std::unique_ptr<pix::Grid> pixels;

    auto pix_data = file.io.read(raw_data_size);
    if (use_palette)
    {
        pixels.reset(new pix::Grid(width, height, pix_data, palette));
        for (auto y : util::range(height))
        for (auto x : util::range(width))
            pixels->at(x, y).a = 0xFF;
    }
    else
    {
        pixels.reset(new pix::Grid(
            width, height, pix_data, pix::Format::BGR888));
    }

    pixels->flip();

    return util::Image::from_pixels(*pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<SotesConverter>("lizsoft/sotes");
