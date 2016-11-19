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

#include "dec/vorbis/packed_ogg_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::vorbis;

static const std::string dir = "tests/dec/vorbis/files/packed_ogg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = PackedOggAudioDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*actual_file, *expected_file, false);
}

TEST_CASE("Vorbis packed OGG audio", "[dec]")
{
    SECTION("Plain")
    {
        do_test("1306.wav", "1306-out.ogg");
    }

    SECTION("Early EOF")
    {
        do_test("90WIF020_001.WAV", "90WIF020_001-out.ogg");
    }
}
