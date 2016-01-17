#include "dec/silky/akb_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::silky;

static const io::path base_dir = "tests/dec/";
static const io::path test_dir = base_dir / "silky/files/akb/";

static void do_test(
    std::unique_ptr<io::File> input_file,
    std::unique_ptr<io::File> expected_file)
{
    const auto decoder = AkbImageDecoder();
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Silky AKB images", "[dec]")
{
    SECTION("24-bit")
    {
        do_test(
            tests::file_from_path(test_dir / "HINT02.AKB"),
            tests::file_from_path(test_dir / "HINT02-out.png"));
    }

    SECTION("32-bit")
    {
        do_test(
            tests::file_from_path(test_dir / "BREATH.AKB"),
            tests::file_from_path(test_dir / "BREATH-out.png"));
    }

    SECTION("AKB+")
    {
        VirtualFileSystem::register_file("homura-base.bmp", []()
            {
                return tests::zlib_file_from_path(
                    test_dir / "homura-base-zlib.akb");
            });
        do_test(
            tests::zlib_file_from_path(test_dir / "homura-overlay-zlib.akb"),
            tests::file_from_path(base_dir / "homura.png"));
    }
}
