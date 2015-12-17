#include "fmt/cat_system/int_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::cat_system;

static const std::string dir = "tests/fmt/cat_system/files/int/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const IntArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("CatSystem INT archives", "[fmt]")
{
    do_test(
        "ptcl.int",
        {
            tests::file_from_path(dir + "ptcl~.int/ase.kcs", "ase.kcs"),
            tests::file_from_path(dir + "ptcl~.int/ase2.kcs", "ase2.kcs"),
            tests::file_from_path(dir + "ptcl~.int/bhole.kcs", "bhole.kcs"),
            tests::file_from_path(dir + "ptcl~.int/bubble.kcs", "bubble.kcs"),
            tests::file_from_path(dir + "ptcl~.int/burst01.kcs", "burst01.kcs"),
            tests::file_from_path(dir + "ptcl~.int/burst02.kcs", "burst02.kcs"),
            tests::file_from_path(dir + "ptcl~.int/burst03.kcs", "burst03.kcs"),
            tests::file_from_path(dir + "ptcl~.int/flare.kcs", "flare.kcs"),
            tests::file_from_path(dir + "ptcl~.int/poison.kcs", "poison.kcs"),
            tests::file_from_path(dir + "ptcl~.int/rain.kcs", "rain.kcs"),
            tests::file_from_path(dir + "ptcl~.int/snow.kcs", "snow.kcs"),
            tests::file_from_path(dir + "ptcl~.int/spark.kcs", "spark.kcs"),
            tests::file_from_path(dir + "ptcl~.int/wind.kcs", "wind.kcs"),
        });
}
