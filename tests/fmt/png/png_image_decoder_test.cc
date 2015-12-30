#include "fmt/png/png_image_decoder.h"
#include <map>
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::png;

static const std::string dir = "tests/fmt/png/files/";

TEST_CASE("PNG images", "[util]")
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

TEST_CASE("PNG images with transparency", "[fmt]")
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

TEST_CASE("PNG images with extra chunks", "[util]")
{
    const auto decoder = PngImageDecoder();
    const auto input_file = tests::file_from_path(dir + "b09s_hs02l_.png");

    SECTION("Default chunk handler")
    {
        REQUIRE_NOTHROW(
            {
                Logger dummy_logger;
                dummy_logger.mute();
                decoder.decode(dummy_logger, *input_file);
            });
    }
    SECTION("Custom chunk handler")
    {
        std::map<std::string, bstr> chunks;
        REQUIRE_NOTHROW(
            {
                Logger dummy_logger;
                dummy_logger.mute();
                decoder.decode(
                    dummy_logger,
                    *input_file,
                    [&](const std::string &name, const bstr &data)
                        {
                            chunks[name] = data;
                        });
            });
        REQUIRE(chunks.size() == 1);
        REQUIRE(chunks["POSn"] == "\x00\x00\x00\x6C\x00\x00\x00\x60"_b);
    }
}
