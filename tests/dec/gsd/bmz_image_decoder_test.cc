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
