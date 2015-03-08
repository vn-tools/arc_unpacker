#include <boost/filesystem.hpp>
#include "compat/main.h"
#include "file_saver.h"
#include "test_support/eassert.h"

void test(const boost::filesystem::path &path, const std::string &name)
{
    FileSaverHdd file_saver(".");

    std::shared_ptr<File> file(new File);
    file->io.write("test");
    file->name = name;

    file_saver.save(file);

    eassert(boost::filesystem::exists(path));
    {
        FileIO file_io(path, FileIOMode::Read);
        eassert(file_io.size() == 4);
        eassert(file_io.read_until_end() == "test");
    }
    boost::filesystem::remove(path);
}

int main(void)
{
    init_fs_utf8();
    // test encoding voodoo
    test("test.out", "test.out");
    test(L"ąćę.out", u8"ąćę.out");
    test(L"不用意な変換.out", u8"不用意な変換.out");

    // observations - boost::filesystem::path:
    // 1. on Linux, it accepts both std::string and std::wstring, and  it works.
    // 1. on Windows,
    // 1.1. can accept std::string paths if a path contains ASCII.
    // 1.2. must be given std::wstring if a path contains non-ASCII. this
    //      std::wstring must be utf16-le encoded.

    // the program assumes utf-8 encoding.
}
