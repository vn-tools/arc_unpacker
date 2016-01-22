#include "dec/purple_software/cpz5_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::purple_software;

static const std::string dir = "tests/dec/purple_software/files/cpz6/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const Cpz5ArchiveDecoder decoder(6);
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Purple Software CPZ6 archives", "[dec]")
{
    do_test(
        "ps.cpz",
        {
            tests::file_from_path(
                dir + "ps~.cpz/maskeffectcut.o2", "shader/maskeffectcut.o2"),
            tests::file_from_path(
                dir + "ps~.cpz/maskeffectput.o2", "shader/maskeffectput.o2"),
            tests::file_from_path(
                dir + "ps~.cpz/transeffect.o2", "shader/transeffect.o2"),
        });
}
