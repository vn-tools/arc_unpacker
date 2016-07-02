#pragma once

#include <memory>
#include <tuple>
#include "io/file.h"
#include "res/image.h"

namespace au {
namespace tests {

    res::Image get_opaque_test_image();
    res::Image get_transparent_test_image();
    std::tuple<res::Image, algo::Grid<int>, res::Palette>
        get_palette_test_image();

    void compare_images(
        const res::Image &actual_image,
        const res::Image &expected_image);

    void compare_images(
        const res::Image &actual_image,
        io::File &expected_file);

    void compare_images(
        io::File &actual_file,
        const res::Image &expected_image);

    void compare_images(
        io::File &actual_file,
        io::File &expected_file,
        const bool compare_file_paths);

    void compare_images(
        const std::vector<std::shared_ptr<io::File>> &actual_files,
        const std::vector<std::shared_ptr<io::File>> &expected_files,
        const bool compare_file_paths);

    void compare_images(
        const std::vector<std::shared_ptr<io::File>> &actual_files,
        const std::vector<res::Image> &expected_images);

    void dump_image(const res::Image &input_image, const io::path &path);

    bool is_image_transparent(const res::Image &image);

    void write_24_bit_image(
        io::BaseByteStream &output_stream, const res::Image &image);

    void write_32_bit_image(
        io::BaseByteStream &output_stream, const res::Image &image);

} }
