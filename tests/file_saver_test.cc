#include <boost/filesystem.hpp>
#include "log.h"
#include "file_saver.h"
#include "test_support/catch.hh"

using namespace au;

static void do_test(const boost::filesystem::path &path)
{
    FileSaverHdd file_saver(".", true);

    auto file = std::make_shared<File>(path.string(), "test"_b);

    Log.mute();
    file_saver.save(file);
    Log.unmute();

    REQUIRE(boost::filesystem::exists(path));
    {
        io::FileIO file_io(path, io::FileMode::Read);
        REQUIRE(file_io.size() == 4);
        REQUIRE(file_io.read_to_eof() == "test"_b);
    }
    boost::filesystem::remove(path);
}

static void do_test_overwriting(
    FileSaver &file_saver1,
    FileSaver &file_saver2,
    bool renamed_file_exists)
{
    boost::filesystem::path path = "test.txt";
    boost::filesystem::path path2 = "test(1).txt";

    auto file = std::make_shared<File>(path.string(), ""_b);

    try
    {
        REQUIRE(!boost::filesystem::exists(path));
        REQUIRE(!boost::filesystem::exists(path2));
        Log.mute();
        file_saver1.save(file);
        file_saver2.save(file);
        Log.unmute();
        REQUIRE(boost::filesystem::exists(path));
        REQUIRE(boost::filesystem::exists(path2) == renamed_file_exists);
        if (boost::filesystem::exists(path)) boost::filesystem::remove(path);
        if (boost::filesystem::exists(path2)) boost::filesystem::remove(path2);
    }
    catch(...)
    {
        if (boost::filesystem::exists(path)) boost::filesystem::remove(path);
        if (boost::filesystem::exists(path2)) boost::filesystem::remove(path2);
        throw;
    }
}

TEST_CASE("Unicode file names", "[core][file_saver]")
{
    do_test("test.out");
    do_test("ąćę.out");
    do_test("不用意な変換.out");
}

TEST_CASE("Two file savers overwrite the same file", "[core][file_saver]")
{
    FileSaverHdd file_saver1(".", true);
    FileSaverHdd file_saver2(".", true);
    do_test_overwriting(file_saver1, file_saver2, false);
}

TEST_CASE("Two file savers don't overwrite the same file", "[core][file_saver]")
{
    FileSaverHdd file_saver1(".", false);
    FileSaverHdd file_saver2(".", false);
    do_test_overwriting(file_saver1, file_saver2, true);
}

TEST_CASE("One file saver never overwrites the same file", "[core][file_saver]")
{
    // even if we pass overwrite=true, files within the same archive with the
    // same name are too valuable to be ovewritten silently
    FileSaverHdd file_saver(".", true);
    do_test_overwriting(file_saver, file_saver, true);
}
