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

#include "dec/kaguya/an00_image_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Atelier Kaguya AN00 image archives", "[dec]")
{
    const std::vector<res::Image> expected_images =
    {
        tests::get_transparent_test_image(),
        tests::get_opaque_test_image(),
    };
    io::File input_file;
    input_file.stream.write("AN00"_b);
    input_file.stream.write_le<u32>(0);
    input_file.stream.write_le<u32>(0);
    input_file.stream.write_le<u32>(expected_images.at(0).height());
    input_file.stream.write_le<u32>(expected_images.at(0).height());
    const std::vector<u32> unk = {1, 2, 3, 4};
    input_file.stream.write_le<u16>(unk.size());
    input_file.stream.write_le<u16>('?');
    for (const auto x : unk)
        input_file.stream.write_le<u32>(x);
    input_file.stream.write_le<u16>(expected_images.size());
    for (const auto &image : expected_images)
    {
        input_file.stream.write_le<u32>('?');
        input_file.stream.write_le<u32>('?');
        input_file.stream.write_le<u32>(image.width());
        input_file.stream.write_le<u32>(image.height());
        res::Image flipped_image(image);
        flipped_image.flip_vertically();
        tests::write_32_bit_image(input_file.stream, flipped_image);
    }

    const auto decoder = An00ImageArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_images(actual_files, expected_images);
}
