#include <boost/filesystem.hpp>
#include "formats/archive.h"
#include "formats/converter.h"
#include "test_support/eassert.h"

typedef boost::filesystem::path path;

namespace
{
    std::string ext(std::string name)
    {
        return boost::filesystem::path(name).extension().string();
    }

    class TestConverter : public Converter
    {
    public:
        std::function<void(File&)> callback;
    protected:
        void decode_internal(File &file) const
        {
            if (callback != nullptr)
                callback(file);
            if (ext(file.name) != ".image")
                throw std::runtime_error("Not an image");
            file.io.truncate(0);
            file.io.write("image");
            file.change_extension("png");
        }
    };

    class TestArchive : public Archive
    {
    public:
        TestConverter test_converter;

        TestArchive()
        {
            add_transformer(&test_converter);
            add_transformer(this);
        }

        void unpack_internal(File &arc_file, FileSaver &file_saver) const
        {
            if (ext(arc_file.name) != ".archive")
                throw std::runtime_error("Not an archive");

            while (!arc_file.io.eof())
            {
                std::unique_ptr<File> output_file(new File);
                output_file->name = arc_file.io.read_until_zero();
                size_t output_file_size = arc_file.io.read_u32_le();
                output_file->io.write(arc_file.io.read(output_file_size));
                file_saver.save(std::move(output_file));
            }
        }
    };

    class FilesystemTestArchive : public Archive
    {
    public:
        FilesystemTestArchive()
        {
        }

        void unpack_internal(File &arc_file, FileSaver &file_saver) const
        {
            auto dir = boost::filesystem::path(arc_file.name).parent_path();
            for (boost::filesystem::directory_iterator it(dir);
                it != boost::filesystem::directory_iterator();
                it ++)
            {
                std::unique_ptr<File> output_file(new File);
                output_file->name = it->path().string();
                file_saver.save(std::move(output_file));
            }
        }
    };

    std::vector<std::shared_ptr<File>> unpack(File &file, Archive &archive)
    {
        std::vector<std::shared_ptr<File>> saved_files;
        FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
        {
            saved_file->io.seek(0);
            saved_files.push_back(saved_file);
        });
        archive.unpack(file, file_saver);
        return saved_files;
    }
}

void test_simple_archive()
{
    TestArchive test_archive;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/file.txt");
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(3);
    dummy_file.io.write("abc");

    std::vector<std::shared_ptr<File>> saved_files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    test_archive.unpack(dummy_file, file_saver);

    eassert(saved_files.size() == 1);
    eassert(path(saved_files[0]->name) == path("deeply/nested/file.txt"));
    eassert(saved_files[0]->io.read_until_end() == "abc");
}

void test_simple_archive_with_converter()
{
    TestArchive test_archive;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/test.image");
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);

    auto saved_files = unpack(dummy_file, test_archive);

    eassert(saved_files.size() == 1);
    eassert(path(saved_files[0]->name) == path("deeply/nested/test.png"));
    eassert(saved_files[0]->io.read_until_end() == "image");
}

void test_converter_receives_full_path()
{
    TestArchive test_archive;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/test.archive");
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    size_t pos = dummy_file.io.tell();
    dummy_file.io.write("further/nested/test.image");
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    size_t sub_archive_size = dummy_file.io.tell() - pos;
    dummy_file.io.seek(pos - 4);
    dummy_file.io.write_u32_le(sub_archive_size);

    std::vector<path> names_passed_to_converter;
    test_archive.test_converter.callback = [&](File &file)
    {
        names_passed_to_converter.push_back(path(file.name));
    };
    unpack(dummy_file, test_archive);

    eassert(names_passed_to_converter.size() == 4);
    eassert(names_passed_to_converter[0] == path("deeply/nested/test.archive"));
    eassert(names_passed_to_converter[1] == path("further/nested/test.image"));
    eassert(names_passed_to_converter[2] == path("further/nested/test.png"));
    eassert(names_passed_to_converter[3]
        == path("deeply/nested/test.archive/further/nested/test.png"));
}

void test_nested_archive()
{
    TestArchive test_archive;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/test.archive");
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    size_t pos = dummy_file.io.tell();
    dummy_file.io.write("further/nested/file.txt");
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    dummy_file.io.write("further/nested/test.image");
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    size_t sub_archive_size = dummy_file.io.tell() - pos;
    dummy_file.io.seek(pos - 4);
    dummy_file.io.write_u32_le(sub_archive_size);

    auto saved_files = unpack(dummy_file, test_archive);

    eassert(saved_files.size() == 2);
    eassert(path(saved_files[0]->name)
        == path("deeply/nested/test.archive/further/nested/file.txt"));
    eassert(saved_files[0]->io.read_until_end() == "");
    eassert(path(saved_files[1]->name)
        == path("deeply/nested/test.archive/further/nested/test.png"));
    eassert(saved_files[1]->io.read_until_end() == "image");
}

void test_archive_filesystem_location()
{
    FilesystemTestArchive test_archive;
    File dummy_file("./tests/formats/transformer_test.cc", FileIOMode::Read);

    auto saved_files = unpack(dummy_file, test_archive);
    eassert(saved_files.size() > 1);

    bool correct = false;
    for (auto &file : saved_files)
    {
        if (path(file->name) == path("./tests/formats/transformer_test.cc"))
            correct = true;
    }
    eassert(correct);
}

int main(void)
{
    test_simple_archive();
    test_simple_archive_with_converter();
    test_nested_archive();
    test_archive_filesystem_location();
    test_converter_receives_full_path();
}
