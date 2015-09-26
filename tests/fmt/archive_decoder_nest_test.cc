#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "fmt/file_decoder.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt;

namespace
{
    using path = boost::filesystem::path;

    class TestFileDecoder final : public FileDecoder
    {
    public:
        std::function<void(File&)> recognition_callback;
        std::function<void(File&)> conversion_callback;
    protected:
        bool is_recognized_internal(File &file) const override;
        std::unique_ptr<File> decode_internal(File &file) const override;
    };

    class TestArchiveDecoder final : public ArchiveDecoder
    {
    public:
        TestFileDecoder test_file_decoder;
        TestArchiveDecoder();
    protected:
        bool is_recognized_internal(File &arc_file) const override;
        void unpack_internal(
            File &arc_file, FileSaver &file_saver) const override;
    };
}

static std::vector<std::shared_ptr<File>> unpack(
    File &file, ArchiveDecoder &archive)
{
    std::vector<std::shared_ptr<File>> saved_files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    archive.unpack(file, file_saver, true);
    return saved_files;
}

bool TestFileDecoder::is_recognized_internal(File &file) const
{
    if (recognition_callback != nullptr)
        recognition_callback(file);
    return file.has_extension("image");
}

std::unique_ptr<File> TestFileDecoder::decode_internal(File &file) const
{
    if (conversion_callback != nullptr)
        conversion_callback(file);
    std::unique_ptr<File> output_file(new File);
    output_file->io.write("image"_b);
    output_file->name = file.name;
    output_file->change_extension("png");
    return output_file;
}

TestArchiveDecoder::TestArchiveDecoder()
{
    add_decoder(&test_file_decoder);
    add_decoder(this);
}

bool TestArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("archive");
}

void TestArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &file_saver) const
{
    while (!arc_file.io.eof())
    {
        std::unique_ptr<File> output_file(new File);
        output_file->name = arc_file.io.read_to_zero().str();
        size_t output_file_size = arc_file.io.read_u32_le();
        output_file->io.write(arc_file.io.read(output_file_size));
        file_saver.save(std::move(output_file));
    }
}

TEST_CASE("Nested files unpack correctly", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/test.image"_b);
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);

    auto saved_files = unpack(dummy_file, test_archive_decoder);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("deeply/nested/test.png"));
    REQUIRE(saved_files[0]->io.read_to_eof() == "image"_b);
}

TEST_CASE("Deeply nested archives unpack correctly", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/test.archive"_b);
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    size_t pos = dummy_file.io.tell();
    dummy_file.io.write("further/nested/file.txt"_b);
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    dummy_file.io.write("further/nested/test.image"_b);
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    size_t sub_archive_size = dummy_file.io.tell() - pos;
    dummy_file.io.seek(pos - 4);
    dummy_file.io.write_u32_le(sub_archive_size);

    auto saved_files = unpack(dummy_file, test_archive_decoder);

    REQUIRE(saved_files.size() == 2);
    REQUIRE(path(saved_files[0]->name)
        == path("deeply/nested/test.archive/further/nested/file.txt"));
    REQUIRE(saved_files[0]->io.read_to_eof() == ""_b);
    REQUIRE(path(saved_files[1]->name)
        == path("deeply/nested/test.archive/further/nested/test.png"));
    REQUIRE(saved_files[1]->io.read_to_eof() == "image"_b);
}

TEST_CASE("Decoder receives full path", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/test.archive"_b);
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    size_t pos = dummy_file.io.tell();
    dummy_file.io.write("further/nested/test.image"_b);
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(0);
    size_t sub_archive_size = dummy_file.io.tell() - pos;
    dummy_file.io.seek(pos - 4);
    dummy_file.io.write_u32_le(sub_archive_size);

    std::vector<path> names_for_recognition;
    std::vector<path> names_for_conversion;
    test_archive_decoder.test_file_decoder.recognition_callback = [&](File &f)
    {
        names_for_recognition.push_back(path(f.name));
    };
    test_archive_decoder.test_file_decoder.conversion_callback = [&](File &f)
    {
        names_for_conversion.push_back(path(f.name));
    };
    unpack(dummy_file, test_archive_decoder);

    REQUIRE(names_for_recognition.size() == 4);
    REQUIRE(names_for_recognition[0] == path("deeply/nested/test.archive"));
    REQUIRE(names_for_recognition[1] == path("further/nested/test.image"));
    REQUIRE(names_for_recognition[2] == path("further/nested/test.png"));
    REQUIRE(names_for_recognition[3]
        == path("deeply/nested/test.archive/further/nested/test.png"));

    REQUIRE(names_for_conversion.size() == 1);
    REQUIRE(names_for_conversion[0] == path("further/nested/test.image"));
}
