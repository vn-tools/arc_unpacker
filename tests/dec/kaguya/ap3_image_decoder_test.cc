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

#include "dec/kaguya/ap3_image_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Kaguya AP3 images", "[dec]")
{
    const auto decoder = Ap3ImageDecoder();
    const auto input_image = tests::get_opaque_test_image();

    io::File input_file;
    input_file.stream.write("AP-3");
    input_file.stream.write_le<u32>(0);
    input_file.stream.write_le<u32>(0);
    input_file.stream.write_le<u32>(input_image.width());
    input_file.stream.write_le<u32>(input_image.height());
    input_file.stream.write_le<u32>(24);

    for (const auto y : algo::range(input_image.height() - 1, -1, -1))
    for (const auto x : algo::range(input_image.width()))
    {
        input_file.stream.write<u8>(input_image.at(x, y).b);
        input_file.stream.write<u8>(input_image.at(x, y).g);
        input_file.stream.write<u8>(input_image.at(x, y).r);
    }

    const auto expected_image = input_image;
    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(actual_image, expected_image);
}
