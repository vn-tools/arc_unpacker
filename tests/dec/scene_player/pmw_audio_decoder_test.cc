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

#include "dec/scene_player/pmw_audio_decoder.h"
#include "algo/binary.h"
#include "algo/pack/zlib.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::scene_player;

TEST_CASE("ScenePlayer PMW audio", "[dec]")
{
    Logger dummy_logger;
    dummy_logger.mute();
    const auto input_text = "some .wav content"_b;
    io::File expected_file("test.wav", input_text);
    io::File input_file(
        "test.pmw",
        algo::unxor(
            algo::pack::zlib_deflate(
                input_text,
                algo::pack::ZlibKind::PlainZlib,
                algo::pack::CompressionLevel::Store),
            0x21));

    const auto decoder = PmwAudioDecoder();
    const auto actual_file = tests::decode(decoder, input_file);
    tests::compare_files(*actual_file, expected_file, true);
}
