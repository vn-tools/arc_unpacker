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

#include "dec/kaguya/an21_image_archive_decoder.h"
#include "algo/range.h"
#include "dec/kaguya/common/rle_test.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

static void mutate_image(res::Image &image)
{
    for (const auto x : algo::range(image.width()))
    for (const auto y : algo::range(image.height()))
    {
        image.at(x, y).r ^= 43;
        image.at(x, y).g ^= 21;
        image.at(x, y).b ^= 99;
    }
}

TEST_CASE("Atelier Kaguya AN21 image archives", "[dec]")
{
    std::vector<res::Image> expected_images =
    {
        tests::get_transparent_test_image(),
        tests::get_transparent_test_image(),
        tests::get_transparent_test_image(),
    };
    mutate_image(expected_images[1]);
    mutate_image(expected_images[2]);

    io::File input_file;

    const std::vector<std::vector<u32>> unk = {
        {0},
        {1, '?', '?'},
        {2, '?'},
        {3, '?'},
        {4, '?'},
        {5, '?'},
    };
    input_file.stream.write("AN21"_b);
    input_file.stream.write_le<u16>(unk.size());
    input_file.stream.write_le<u16>('?');
    for (const auto x : unk)
    {
        input_file.stream.write<u8>(x[0]);
        for (const auto i : algo::range(1, x.size()))
            input_file.stream.write_le<u32>(x[i]);
    }

    const std::vector<std::pair<u32, u32>> unk2 = {{1, 2}, {3, 4}};
    input_file.stream.write_le<u16>(unk2.size());
    for (const auto x : unk2)
    {
        input_file.stream.write_le<u32>(x.first);
        input_file.stream.write_le<u32>(x.second);
    }
    input_file.stream.write("[PIC]10"_b);

    input_file.stream.write_le<u16>(expected_images.size());
    input_file.stream.write_le<u32>(0);
    input_file.stream.write_le<u32>(0);
    input_file.stream.write_le<u32>(expected_images.at(0).width());
    input_file.stream.write_le<u32>(expected_images.at(0).height());

    bstr last_data;

    {
        const auto &base_image = expected_images.at(0);
        input_file.stream.write_le<u32>(0);
        input_file.stream.write_le<u32>(0);
        input_file.stream.write_le<u32>(base_image.width());
        input_file.stream.write_le<u32>(base_image.height());
        input_file.stream.write_le<u32>(4);

        res::Image flipped_image(base_image);
        flipped_image.flip_vertically();
        tests::write_32_bit_image(input_file.stream, flipped_image);

        io::MemoryByteStream tmp_stream;
        tests::write_32_bit_image(tmp_stream, flipped_image);
        last_data = tmp_stream.seek(0).read_to_eof();
    }

    for (const auto i : algo::range(1, expected_images.size()))
    {
        res::Image flipped_image(expected_images[i]);
        flipped_image.flip_vertically();

        io::MemoryByteStream tmp_stream;
        tests::write_32_bit_image(tmp_stream, flipped_image);
        auto data = tmp_stream.seek(0).read_to_eof();

        auto diff_data = data;
        for (const auto j : algo::range(data.size()))
            diff_data[j] -= last_data[j];

        const auto compressed_data
            = tests::dec::kaguya::common::compress_rle(diff_data);
        input_file.stream.write<u8>(4);
        input_file.stream.write_le<u32>(compressed_data.size());
        input_file.stream.write(compressed_data);
        last_data = data;
    }

    const auto decoder = An21ImageArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_images(actual_files, expected_images);
}
