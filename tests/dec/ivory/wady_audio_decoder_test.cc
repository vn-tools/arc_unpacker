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

#include "dec/ivory/wady_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::ivory;

static const std::string dir = "tests/dec/ivory/files/wady/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = WadyAudioDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::zlib_file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(actual_audio, *expected_file);
}

TEST_CASE("Ivory WADY audio", "[dec]")
{
    SECTION("Version 1 (uncompressed), stereo")
    {
        do_test("m01-zlib", "m01-zlib-out.wav");
    }

    SECTION("Version 2 (compressed), mono")
    {
        do_test("10510-zlib", "10510-zlib-out.wav");
    }

    SECTION("Version 2 (compressed), stereo")
    {
        do_test("071-zlib", "071-zlib-out.wav");
    }
}
