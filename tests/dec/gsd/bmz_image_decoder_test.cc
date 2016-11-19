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

#include "dec/gsd/bmz_image_decoder.h"
#include "algo/pack/zlib.h"
#include "enc/microsoft/bmp_image_encoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::gsd;

TEST_CASE("GSD BMZ images", "[dec]")
{
    const auto expected_image = tests::get_transparent_test_image();

    Logger dummy_logger;
    dummy_logger.mute();
    const auto bmp_encoder = enc::microsoft::BmpImageEncoder();
    const auto bmp_file = bmp_encoder.encode(
        dummy_logger, expected_image, "test.bmp");

    const auto data_orig = bmp_file->stream.seek(0).read_to_eof();
    const auto data_comp = algo::pack::zlib_deflate(
        data_orig,
        algo::pack::ZlibKind::PlainZlib,
        algo::pack::CompressionLevel::Best);

    io::File input_file;
    input_file.stream.write("ZLC3"_b);
    input_file.stream.write_le<u32>(data_orig.size());
    input_file.stream.write(data_comp);

    const auto decoder = BmzImageDecoder();
    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(actual_image, expected_image);
}
