#include <boost/filesystem.hpp>
#include "formats/archive.h"
#include "formats/converter.h"
#include "test_support/catch.hpp"

typedef boost::filesystem::path path;

namespace
{
    class TestConverter : public Converter
    {
    public:
        std::function<void(File&)> recognition_callback;
        std::function<void(File&)> conversion_callback;
    protected:
        bool is_recognized_internal(File &file) const override
        {
            if (recognition_callback != nullptr)
                recognition_callback(file);
            return file.has_extension("image");
        }

        std::unique_ptr<File> decode_internal(File &file) const override
        {
            if (conversion_callback != nullptr)
                conversion_callback(file);
            std::unique_ptr<File> output_file(new File);
            output_file->io.write("image");
            output_file->name = file.name;
            output_file->change_extension("png");
            return output_file;
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

        bool is_recognized_internal(File &arc_file) const override
        {
            return arc_file.has_extension("archive");
        }

        void unpack_internal(
            File &arc_file, FileSaver &file_saver) const override
        {
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

        bool is_recognized_internal(File &arc_file) const override
        {
            return true;
        }

        void unpack_internal(
            File &arc_file, FileSaver &file_saver) const override
        {
            auto dir = boost::filesystem::path(arc_file.name).parent_path();
            for (boost::filesystem::directory_iterator it(dir);
                it != boost::filesystem::directory_iterator();
                it++)
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

TEST_CASE("Simple archive unpacks correctly")
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

    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("deeply/nested/file.txt"));
    REQUIRE(saved_files[0]->io.read_until_end() == "abc");
}

TEST_CASE("Simple archive with converter unpacks correctly")
{
    TestArchive test_archive;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/test.image");
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);

    auto saved_files = unpack(dummy_file, test_archive);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("deeply/nested/test.png"));
    REQUIRE(saved_files[0]->io.read_until_end() == "image");
}

TEST_CASE("Converter receives full path")
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

    std::vector<path> names_for_recognition;
    std::vector<path> names_for_conversion;
    test_archive.test_converter.recognition_callback = [&](File &file)
    {
        names_for_recognition.push_back(path(file.name));
    };
    test_archive.test_converter.conversion_callback = [&](File &file)
    {
        names_for_conversion.push_back(path(file.name));
    };
    unpack(dummy_file, test_archive);

    REQUIRE(names_for_recognition.size() == 4);
    REQUIRE(names_for_recognition[0] == path("deeply/nested/test.archive"));
    REQUIRE(names_for_recognition[1] == path("further/nested/test.image"));
    REQUIRE(names_for_recognition[2] == path("further/nested/test.png"));
    REQUIRE(names_for_recognition[3]
        == path("deeply/nested/test.archive/further/nested/test.png"));

    REQUIRE(names_for_conversion.size() == 1);
    REQUIRE(names_for_conversion[0] == path("further/nested/test.image"));
}

TEST_CASE("Nested archives unpack correctly")
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

    REQUIRE(saved_files.size() == 2);
    REQUIRE(path(saved_files[0]->name)
        == path("deeply/nested/test.archive/further/nested/file.txt"));
    REQUIRE(saved_files[0]->io.read_until_end() == "");
    REQUIRE(path(saved_files[1]->name)
        == path("deeply/nested/test.archive/further/nested/test.png"));
    REQUIRE(saved_files[1]->io.read_until_end() == "image");
}

TEST_CASE("Files get correct location")
{
    FilesystemTestArchive test_archive;
    File dummy_file("./tests/formats/transformer_test.cc", FileIOMode::Read);

    auto saved_files = unpack(dummy_file, test_archive);
    REQUIRE(saved_files.size() > 1);

    bool correct = false;
    for (auto &file : saved_files)
    {
        if (path(file->name) == path("./tests/formats/transformer_test.cc"))
            correct = true;
    }
    REQUIRE(correct);
}
