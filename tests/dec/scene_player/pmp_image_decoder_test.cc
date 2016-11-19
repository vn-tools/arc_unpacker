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

#include "dec/scene_player/pmp_image_decoder.h"
#include "algo/binary.h"
#include "algo/pack/zlib.h"
#include "enc/microsoft/bmp_image_encoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::scene_player;

TEST_CASE("ScenePlayer PMP images", "[dec]")
{
    Logger dummy_logger;
    dummy_logger.mute();
    const auto input_image = tests::get_opaque_test_image();
    const auto bmp_encoder = enc::microsoft::BmpImageEncoder();
    const auto bmp_file
        = bmp_encoder.encode(dummy_logger, input_image, "test.dat");
    io::File input_file(
        "test.pmp",
        algo::unxor(
            algo::pack::zlib_deflate(
                bmp_file->stream.seek(0).read_to_eof(),
                algo::pack::ZlibKind::PlainZlib,
                algo::pack::CompressionLevel::Store),
            0x21));

    const auto decoder = PmpImageDecoder();
    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(actual_image, input_image);
}
