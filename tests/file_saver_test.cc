#include <boost/filesystem.hpp>
#include "compat/main.h"
#include "file_saver.h"
#include "test_support/eassert.h"

void test(const boost::filesystem::path &path)
{
    FileSaverHdd file_saver(".");

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

int main(void)
{
    init_fs_utf8();
    // test encoding voodoo
    test("test.out");
    test("ąćę.out");
    test("不用意な変換.out");
}
