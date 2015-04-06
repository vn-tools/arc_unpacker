#include <boost/filesystem.hpp>
#include "compat/main.h"
#include "file_saver.h"
#include "test_support/eassert.h"
#include "test_support/suppress_output.h"

void test(const boost::filesystem::path &path)
{
    FileSaverHdd file_saver(".", true);

    std::shared_ptr<File> file(new File);
    file->io.write("test");
    file->name = path.string();

    file_saver.save(file);

    eassert(boost::filesystem::exists(path));
    {
        FileIO file_io(path, FileIOMode::Read);
        eassert(file_io.size() == 4);
        eassert(file_io.read_until_end() == "test");
    }
    boost::filesystem::remove(path);
}

void do_test_overwriting(
    FileSaver &file_saver1,
    FileSaver &file_saver2,
    bool renamed_file_exists)
{
    boost::filesystem::path path = "test.txt";
    boost::filesystem::path path2 = "test(1).txt";

    std::shared_ptr<File> file(new File);
    file->name = path.string();

    try
    {
        eassert(!boost::filesystem::exists(path));
        eassert(!boost::filesystem::exists(path2));
        file_saver1.save(file);
        file_saver2.save(file);
        eassert(boost::filesystem::exists(path));
        eassert(boost::filesystem::exists(path2) == renamed_file_exists);
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

void test_overwriting_two_file_savers()
{
    FileSaverHdd file_saver1(".", true);
    FileSaverHdd file_saver2(".", true);
    do_test_overwriting(file_saver1, file_saver2, false);
}

void test_not_overwriting_two_file_savers()
{
    FileSaverHdd file_saver1(".", false);
    FileSaverHdd file_saver2(".", false);
    do_test_overwriting(file_saver1, file_saver2, true);
}

void test_not_overwriting_one_file_saver()
{
    //even if we pass overwrite=true, files within the same archive with the
    //same name are too valuable to be ovewritten silently
    FileSaverHdd file_saver(".", true);
    do_test_overwriting(file_saver, file_saver, true);
}

int main(void)
{
    suppress_output([&]()
    {
        // test encoding voodoo
        init_fs_utf8();
        test("test.out");
        test("ąćę.out");
        test("不用意な変換.out");

        //test file overwriting
        test_overwriting_two_file_savers();
        test_not_overwriting_two_file_savers();
        test_not_overwriting_one_file_saver();
    });
}
