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
