#include "dec/cri/xtx_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::cri;

static const io::path cps_dir = "tests/dec/cri/files/cps/";
static const io::path xtx_dir = "tests/dec/cri/files/xtx/";

static void do_test(
    const io::path &input_path, const io::path &expected_path)
{
    const auto decoder = XtxImageDecoder();
    const auto input_file = tests::zlib_file_from_path(input_path);
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("CRI XTX images", "[dec]")
{
    do_test(xtx_dir / "unk_1186-zlib.xtx", xtx_dir / "unk_1186-out.png");
}
