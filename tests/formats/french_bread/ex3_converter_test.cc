#include "formats/french_bread/ex3_converter.h"
#include "io/file_io.h"
#include "test_support/catch.hpp"
#include "util/zlib.h"
using namespace Formats::FrenchBread;

TEST_CASE("Decoding EX3 images works")
{
    Ex3Converter converter;
    const std::string input_path
        = "tests/formats/french_bread/files/WIN_HISUI&KOHAKU.EX3";
    const std::string expected_path
        = "tests/formats/french_bread/files/WIN_HISUI&KOHAKU-out.bmpz";

    FileIO input_io(input_path, FileIOMode::Read);
    File file;
    file.io.write_from_io(input_io, input_io.size());
    auto output_file = converter.decode(file);

    FileIO expected_io(expected_path, FileIOMode::Read);
    const auto expected_data = zlib_inflate(expected_io.read_until_end());

    output_file->io.seek(0);
    REQUIRE(expected_data.size() == output_file->io.size());
    REQUIRE(expected_data == output_file->io.read_until_end());
}
