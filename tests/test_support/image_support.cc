// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "test_support/image_support.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/jpeg/jpeg_image_decoder.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "enc/png/png_image_encoder.h"
#include "test_support/catch.h"
#include "test_support/common.h"
#include "test_support/file_support.h"

using namespace au;

static inline bool compare_pixels(
    const res::Pixel actual,
    const res::Pixel expected,
    const size_t x,
    const size_t y)
{
    if (actual != expected)
    {
        if (actual.a == 0 && expected.a == 0)
            return true;
        WARN(algo::format(
            "Pixels differ at %d, %d: %02x%02x%02x%02x != %02x%02x%02x%02x",
            x, y,
            actual.b, actual.g, actual.r, actual.a,
            expected.b, expected.g, expected.r, expected.a));
        return false;
    }
    return true;
}

static res::Image image_from_file(io::File &file)
{
    Logger dummy_logger;
    dummy_logger.mute();

    static const auto png_image_decoder = dec::png::PngImageDecoder();
    if (png_image_decoder.is_recognized(file))
        return png_image_decoder.decode(dummy_logger, file);

    static const auto bmp_image_decoder = dec::microsoft::BmpImageDecoder();
    if (bmp_image_decoder.is_recognized(file))
        return bmp_image_decoder.decode(dummy_logger, file);

    static const auto jpeg_image_decoder = dec::jpeg::JpegImageDecoder();
    if (jpeg_image_decoder.is_recognized(file))
        return jpeg_image_decoder.decode(dummy_logger, file);

    throw std::logic_error("Only PNG, BMP and JPEG files are supported");
}

res::Image tests::get_opaque_test_image()
{
    const auto input_file = tests::file_from_path("tests/dec/homura.png");
    return image_from_file(*input_file);
}

res::Image tests::get_transparent_test_image()
{
    const auto input_file = tests::file_from_path(
        "tests/dec/png/files/reimu_transparent.png");
    return image_from_file(*input_file);
}

std::tuple<res::Image, algo::Grid<int>, res::Palette>
    tests::get_palette_test_image()
{
    auto input_image = tests::get_opaque_test_image();
    res::Palette output_palette(256);
    for (const auto i : algo::range(256))
    {
        // make colors a bit funky so that tests catch channel-wise mistakes
        output_palette[i].r = 255 - i;
        output_palette[i].g = 254 - i;
        output_palette[i].b = 253 - i;
        output_palette[i].a = i;
    }

    algo::Grid<int> output_grid(input_image.width(), input_image.height());
    res::Image output_image(input_image.width(), input_image.height());
    for (const auto y : algo::range(input_image.height()))
    for (const auto x : algo::range(input_image.width()))
    {
        const auto &input_pixel = input_image.at(x, y);
        const u8 palette_index = 255 - ((
            input_pixel.r +
            input_pixel.g +
            input_pixel.b) / 3);

        auto &output_pixel = output_image.at(x, y);
        output_pixel.b = output_palette[palette_index].b;
        output_pixel.g = output_palette[palette_index].g;
        output_pixel.r = output_palette[palette_index].r;
        output_pixel.a = output_palette[palette_index].a;

        output_grid.at(x, y) = palette_index;
    }
    return std::make_tuple(output_image, output_grid, output_palette);
}

void tests::compare_images(
    const res::Image &actual_image, const res::Image &expected_image)
{
    REQUIRE(actual_image.width() == expected_image.width());
    REQUIRE(actual_image.height() == expected_image.height());

    bool images_match = true;
    for (const auto y : algo::range(expected_image.height()))
    for (const auto x : algo::range(expected_image.width()))
    {
        const auto expected_pixel = expected_image.at(x, y);
        const auto actual_pixel = actual_image.at(x, y);
        images_match &= compare_pixels(actual_pixel, expected_pixel, x, y);
    }
    REQUIRE(images_match);
}

void tests::compare_images(
    const res::Image &actual_image, io::File &expected_file)
{
    tests::compare_images(actual_image, image_from_file(expected_file));
}

void tests::compare_images(
    io::File &actual_file, const res::Image &expected_image)
{
    tests::compare_images(image_from_file(actual_file), expected_image);
}

void tests::compare_images(
    io::File &actual_file,
    io::File &expected_file,
    const bool compare_file_paths)
{
    auto expected_image = image_from_file(expected_file);
    auto actual_image = image_from_file(actual_file);
    if (compare_file_paths)
        tests::compare_paths(actual_file.path, expected_file.path);
    tests::compare_images(actual_image, expected_image);
}

void tests::compare_images(
    const std::vector<std::shared_ptr<io::File>> &actual_files,
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bool compare_file_paths)
{
    REQUIRE(actual_files.size() == expected_files.size());
    for (const auto i : algo::range(expected_files.size()))
    {
        INFO(algo::format("Images at index %d differ", i));
        tests::compare_images(
            *actual_files[i], *expected_files[i], compare_file_paths);
    }
}

void tests::compare_images(
    const std::vector<std::shared_ptr<io::File>> &actual_files,
    const std::vector<res::Image> &expected_images)
{
    REQUIRE(actual_files.size() == expected_images.size());
    for (const auto i : algo::range(expected_images.size()))
    {
        INFO(algo::format("Images at index %d differ", i));
        tests::compare_images(*actual_files[i], expected_images[i]);
    }
}

void tests::dump_image(const res::Image &input_image, const io::path &path)
{
    Logger dummy_logger;
    dummy_logger.mute();

    enc::png::PngImageEncoder encoder;
    const auto output_file = encoder.encode(dummy_logger, input_image, path);
    const auto data = output_file->stream.seek(0).read_to_eof();
    io::File(path, io::FileMode::Write).stream.write(data);
}

bool tests::is_image_transparent(const res::Image &image)
{
    for (const auto &c : image)
        if (c.a != 0xFF)
            return true;
    return false;
}

void tests::write_32_bit_image(
    io::BaseByteStream &output_stream, const res::Image &image)
{
    for (const auto y : algo::range(image.height()))
    for (const auto x : algo::range(image.width()))
    {
        output_stream.write<u8>(image.at(x, y).b);
        output_stream.write<u8>(image.at(x, y).g);
        output_stream.write<u8>(image.at(x, y).r);
        output_stream.write<u8>(image.at(x, y).a);
    }
}

void tests::write_24_bit_image(
    io::BaseByteStream &output_stream, const res::Image &image)
{
    for (const auto y : algo::range(image.height()))
    for (const auto x : algo::range(image.width()))
    {
        output_stream.write<u8>(image.at(x, y).b);
        output_stream.write<u8>(image.at(x, y).g);
        output_stream.write<u8>(image.at(x, y).r);
    }
}
