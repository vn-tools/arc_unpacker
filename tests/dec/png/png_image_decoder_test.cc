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

#include "dec/png/png_image_decoder.h"
#include <map>
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::png;

static const std::string dir = "tests/dec/png/files/";

TEST_CASE("PNG images", "[dec]")
{
    Logger dummy_logger;
    dummy_logger.mute();
    io::File file(dir + "usagi_opaque.png", io::FileMode::Read);

    const auto decoder = PngImageDecoder();
    const auto image = decoder.decode(dummy_logger, file);
    REQUIRE(image.width() == 640);
    REQUIRE(image.height() == 480);

    const auto color = image.at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0x7C);
    REQUIRE(static_cast<int>(color.g) == 0x6A);
    REQUIRE(static_cast<int>(color.b) == 0x34);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}

TEST_CASE("PNG images with transparency", "[dec]")
{
    Logger dummy_logger;
    dummy_logger.mute();
    io::File file(dir + "reimu_transparent.png", io::FileMode::Read);

    const auto decoder = PngImageDecoder();
    const auto image = decoder.decode(dummy_logger, file);
    REQUIRE(image.width() == 641);
    REQUIRE(image.height() == 720);

    const auto color = image.at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0xFE);
    REQUIRE(static_cast<int>(color.g) == 0x0A);
    REQUIRE(static_cast<int>(color.b) == 0x17);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}

TEST_CASE("PNG images with extra chunks", "[dec]")
{
    const auto decoder = PngImageDecoder();
    const auto input_file = tests::file_from_path(dir + "b09s_hs02l_.png");

    SECTION("Default chunk handler")
    {
        REQUIRE_NOTHROW(
            [&]()
            {
                Logger dummy_logger;
                dummy_logger.mute();
                decoder.decode(dummy_logger, *input_file);
            }());
    }

    SECTION("Custom chunk handler")
    {
        std::map<std::string, bstr> chunks;
        REQUIRE_NOTHROW(
            [&]()
            {
                Logger dummy_logger;
                dummy_logger.mute();
                decoder.decode(
                    dummy_logger,
                    *input_file,
                    [&chunks](const std::string &name, const bstr &data)
                    {
                        chunks[name] = data;
                    });
            }());
        REQUIRE(chunks.size() == 1);
        REQUIRE(chunks["POSn"] == "\x00\x00\x00\x6C\x00\x00\x00\x60"_b);
    }
}
