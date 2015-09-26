#include "fmt/touhou/pak1_audio_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Touhou PAK1 audio", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path("tests/fmt/touhou/files/pak1/se-0001-out.wav"),
        tests::file_from_path("tests/fmt/touhou/files/pak1/se-0002-out.wav"),
    };
    expected_files[0]->name = "0001.wav";
    expected_files[1]->name = "0002.wav";

    Pak1AudioArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/touhou/files/pak1/se.dat", decoder);

    tests::compare_files(expected_files, actual_files, true);
}
