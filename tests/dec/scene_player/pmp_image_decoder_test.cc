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
