#include "file_io.h"
#include "formats/gfx/ex3_converter.h"
#include "string_ex.h"
#include "test_support/eassert.h"

void test_ex3_decoding()
{
    Ex3Converter converter;
    const std::string input_path
        = "tests/test_files/gfx/ex3/WIN_HISUI&KOHAKU.EX3";
    const std::string expected_path
        = "tests/test_files/gfx/ex3/WIN_HISUI&KOHAKU-out.bmpz";

    FileIO input_io(input_path, "rb");
    VirtualFile file;
    file.io.write_from_io(input_io, input_io.size());
    converter.decode(file);

    FileIO expected_io(expected_path, "rb");
    const std::string expected_data
        = zlib_inflate(expected_io.read(expected_io.size()));

    file.io.seek(0);
    eassert(expected_data.size() == file.io.size());
    eassert(expected_data == file.io.read(file.io.size()));
}

int main(void)
{
    test_ex3_decoding();
    return 0;
}

