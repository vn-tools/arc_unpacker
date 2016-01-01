#include "dec/purple_software/cpz5_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::purple_software;

static const std::string dir = "tests/dec/purple_software/files/cpz5/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const Cpz5ArchiveDecoder decoder(5);
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Purple Software CPZ5 archives", "[dec]")
{
    SECTION("Variant 1")
    {
        do_test(
            "v1_ps.cpz",
            {
                tests::file_from_path(
                    dir + "v1_ps~.cpz/maskeffectcut.o2", "maskeffectcut.o2"),
                tests::file_from_path(
                    dir + "v1_ps~.cpz/maskeffectput.o2", "maskeffectput.o2"),
            });
    }

    SECTION("Variant 2")
    {
        do_test(
            "v2_ps.cpz",
            {
                tests::file_from_path(
                    dir + "v2_ps~.cpz/transeffect.o2", "transeffect.o2"),
                tests::file_from_path(
                    dir + "v2_ps~.cpz/maskeffectcut.o2", "maskeffectcut.o2"),
                tests::file_from_path(
                    dir + "v2_ps~.cpz/maskeffectput.o2", "maskeffectput.o2"),
            });
    }
}
