#include "fmt/french_bread/ex3_converter.h"
#include "io/file_io.h"
#include "test_support/catch.hpp"
#include "util/zlib.h"

using namespace au;
using namespace au::fmt::french_bread;

TEST_CASE("Decoding EX3 images works")
{
    Ex3Converter converter;
    const std::string input_path
        = "tests/fmt/french_bread/files/WIN_HISUI&KOHAKU.EX3";
    const std::string expected_path
        = "tests/fmt/french_bread/files/WIN_HISUI&KOHAKU-out.bmpz";

    io::FileIO input_io(input_path, io::FileMode::Read);
    File file;
    file.io.write_from_io(input_io, input_io.size());
    auto output_file = converter.decode(file);

    io::FileIO expected_io(expected_path, io::FileMode::Read);
    const auto expected_data = au::util::zlib_inflate(
        expected_io.read_until_end());

    output_file->io.seek(0);
    REQUIRE(expected_data.size() == output_file->io.size());
    REQUIRE(expected_data == output_file->io.read_until_end());
}
