#include "enc/microsoft/bmp_image_encoder.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::enc::microsoft;

TEST_CASE("Microsoft BMP images encoding", "[enc]")
{
    Logger dummy_logger;
    dummy_logger.mute();
    const auto bmp_encoder = BmpImageEncoder();

    SECTION("Small image")
    {
        res::Image input_image(1, 1);
        input_image.at(0, 0).r = 1;
        input_image.at(0, 0).g = 2;
        input_image.at(0, 0).b = 3;
        input_image.at(0, 0).a = 0xFF;
        const auto output_file
            = bmp_encoder.encode(dummy_logger, input_image, "test.dat");
        REQUIRE(output_file->path.name() == "test.bmp");
        REQUIRE(output_file->stream.seek(0).read_to_eof() ==
            "\x42\x4D\x3A\x00\x00\x00\x00\x00\x00\x00\x36\x00\x00\x00\x28\x00"
            "\x00\x00\x01\x00\x00\x00\xFF\xFF\xFF\xFF\x01\x00\x18\x00\x00\x00"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\x00\x00\x00\x00\x00\x00\x03\x02\x01\x00"_b);
    }

    SECTION("Bigger image")
    {
        const auto input_file = tests::file_from_path("tests/dec/homura.png");
        const auto png_decoder = dec::png::PngImageDecoder();
        const auto bmp_decoder = dec::microsoft::BmpImageDecoder();
        const auto input_image = png_decoder.decode(dummy_logger, *input_file);
        const auto output_file
            = bmp_encoder.encode(dummy_logger, input_image, "test.dat");
        const auto output_image
            = bmp_decoder.decode(dummy_logger, *output_file);
        tests::compare_images(input_image, output_image);
    }
}
