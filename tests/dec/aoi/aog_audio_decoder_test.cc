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

#include "dec/aoi/aog_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::aoi;

TEST_CASE("Aoi AOG audio files", "[dec]")
{
    SECTION("With Aoi header")
    {
        const auto decoder = AogAudioDecoder();
        io::File input_file(
            "test.aog",
            "AoiOgg\x00\x00JUNKJUNKJUNKJUNKJUNKJUNKJUNKJUNKJUNKOggSwhatever"_b);
        const io::File expected_file("test.ogg", "OggSwhatever"_b);
        const auto actual_file = tests::decode(decoder, input_file);
        tests::compare_files(*actual_file, expected_file, true);
    }

    SECTION("Without Aoi header")
    {
        const auto decoder = AogAudioDecoder();
        io::File input_file("test.aog", "OggSwhatever"_b);
        const io::File expected_file("test.ogg", "OggSwhatever"_b);
        const auto actual_file = tests::decode(decoder, input_file);
        tests::compare_files(*actual_file, expected_file, true);
    }
}
