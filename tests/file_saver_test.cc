#include "file_saver.h"
#include "io/file_system.h"
#include "test_support/catch.h"

using namespace au;

static void do_test(const io::path &path)
{
    const FileSaverHdd file_saver(".", true);
    const auto file = std::make_shared<io::File>(path.str(), "test"_b);

    file_saver.save(file);

    REQUIRE(io::exists(path));
    {
        io::FileStream file_stream(path, io::FileMode::Read);
        REQUIRE(file_stream.size() == 4);
        REQUIRE(file_stream.read_to_eof() == "test"_b);
    }
    io::remove(path);
}

static void do_test_overwriting(
    const FileSaver &file_saver1,
    const FileSaver &file_saver2,
    const bool renamed_file_exists)
{
    io::path path = "test.txt";
    io::path path2 = "test(1).txt";
    const auto file = std::make_shared<io::File>(path.str(), ""_b);

    try
    {
        REQUIRE(!io::exists(path));
        REQUIRE(!io::exists(path2));
        file_saver1.save(file);
        file_saver2.save(file);
        REQUIRE(io::exists(path));
        REQUIRE(io::exists(path2) == renamed_file_exists);
        if (io::exists(path)) io::remove(path);
        if (io::exists(path2)) io::remove(path2);
    }
    catch(...)
    {
        if (io::exists(path)) io::remove(path);
        if (io::exists(path2)) io::remove(path2);
        throw;
    }
}

TEST_CASE("FileSaver", "[core]")
{
    SECTION("Unicode file names")
    {
        do_test("test.out");
        do_test(u8"ąćę.out");
        do_test(u8"不用意な変換.out");
    }

    SECTION("Two file savers overwrite the same file")
    {
        const FileSaverHdd file_saver1(".", true);
        const FileSaverHdd file_saver2(".", true);
        do_test_overwriting(file_saver1, file_saver2, false);
    }

    SECTION("Two file savers don't overwrite the same file")
    {
        const FileSaverHdd file_saver1(".", false);
        const FileSaverHdd file_saver2(".", false);
        do_test_overwriting(file_saver1, file_saver2, true);
    }

    SECTION("One file saver never overwrites the same file")
    {
        // even if we pass overwrite=true, files within the same archive with
        // the same name are too valuable to be ovewritten silently
        const FileSaverHdd file_saver(".", true);
        do_test_overwriting(file_saver, file_saver, true);
    }
}
