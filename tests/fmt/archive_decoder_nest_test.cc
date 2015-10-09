#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "fmt/file_decoder.h"
#include "io/buffered_io.h"
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

    struct ArchiveEntryImpl final : ArchiveEntry
    {
        size_t offset;
        size_t size;
    };

    class TestArchiveDecoder final : public ArchiveDecoder
    {
    public:
        TestFileDecoder test_file_decoder;
        TestArchiveDecoder();
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    protected:
        bool is_recognized_internal(File &arc_file) const override;
    };
}

static std::vector<std::shared_ptr<File>> unpack(
    File &file, ArchiveDecoder &decoder)
{
    std::vector<std::shared_ptr<File>> saved_files;
    FileSaverCallback saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    decoder.unpack(file, saver);
    return saved_files;
}

static bstr serialize_file(const std::string &name, const bstr &content)
{
    io::BufferedIO io;
    io.write(name);
    io.write_u8(0);
    io.write_u32_le(content.size());
    io.write(content);
    io.seek(0);
    return io.read_to_eof();
}

bool TestFileDecoder::is_recognized_internal(File &file) const
{
    if (recognition_callback)
        recognition_callback(file);
    return file.has_extension("image");
}

std::unique_ptr<File> TestFileDecoder::decode_internal(File &file) const
{
    if (conversion_callback)
        conversion_callback(file);
    auto output_file = std::make_unique<File>();
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

std::unique_ptr<ArchiveMeta>
    TestArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(0);
    auto meta = std::make_unique<ArchiveMeta>();
    while (!arc_file.io.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero().str();
        entry->size = arc_file.io.read_u32_le();
        entry->offset = arc_file.io.tell();
        arc_file.io.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> TestArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    return std::make_unique<File>(entry->name, arc_file.io.read(entry->size));
}

TEST_CASE("Nested files unpack correctly", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;
    File test_archive_file;
    test_archive_file.name = "test.archive";
    test_archive_file.io.write(serialize_file("test.image", ""_b));

    auto saved_files = unpack(test_archive_file, test_archive_decoder);
    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("test.png"));
    REQUIRE(saved_files[0]->io.read_to_eof() == "image"_b);
}

TEST_CASE("Recursively nested files unpack correctly", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;

    io::BufferedIO nested_archive_io;
    nested_archive_io.write(serialize_file("nested/file.txt", ""_b));
    nested_archive_io.write(serialize_file("nested/test.image", ""_b));
    nested_archive_io.seek(0);
    auto nested_archive_content = nested_archive_io.read_to_eof();

    File test_archive_file;
    test_archive_file.name = "test.archive";
    test_archive_file.io.write(
        serialize_file("test.archive", nested_archive_content));

    auto saved_files = unpack(test_archive_file, test_archive_decoder);
    REQUIRE(saved_files.size() == 2);
    REQUIRE(path(saved_files[0]->name) == path("test.archive/nested/file.txt"));
    REQUIRE(path(saved_files[1]->name) == path("test.archive/nested/test.png"));
    REQUIRE(saved_files[0]->io.read_to_eof() == ""_b);
    REQUIRE(saved_files[1]->io.read_to_eof() == "image"_b);
}

TEST_CASE("Disabling recursive decoding", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;
    test_archive_decoder.disable_nested_decoding();

    io::BufferedIO nested_archive_io;
    nested_archive_io.write(serialize_file("nested/file.txt", ""_b));
    nested_archive_io.write(serialize_file("nested/test.image", ""_b));
    nested_archive_io.seek(0);
    auto nested_archive_content = nested_archive_io.read_to_eof();

    File test_archive_file;
    test_archive_file.name = "test.archive";
    test_archive_file.io.write(
        serialize_file("test.archive", nested_archive_content));

    auto saved_files = unpack(test_archive_file, test_archive_decoder);
    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("test.archive"));
    REQUIRE(saved_files[0]->io.read_to_eof() == nested_archive_content);
}

TEST_CASE("Internal decoders receive full paths", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;

    io::BufferedIO nested_archive_io;
    nested_archive_io.write(serialize_file("nested/test.image", ""_b));
    nested_archive_io.seek(0);
    auto nested_archive_content = nested_archive_io.read_to_eof();

    File test_archive_file;
    test_archive_file.name = "test.archive";
    test_archive_file.io.write(
        serialize_file("test.archive", nested_archive_content));

    std::vector<path> names_for_recognition;
    std::vector<path> names_for_conversion;
    test_archive_decoder.test_file_decoder.recognition_callback = [&](File &f)
        { names_for_recognition.push_back(path(f.name)); };
    test_archive_decoder.test_file_decoder.conversion_callback = [&](File &f)
        { names_for_conversion.push_back(path(f.name)); };
    unpack(test_archive_file, test_archive_decoder);

    REQUIRE(names_for_recognition.size() == 4);
    REQUIRE(names_for_recognition[0] == path("test.archive"));
    REQUIRE(names_for_recognition[1] == path("nested/test.image"));
    REQUIRE(names_for_recognition[2] == path("nested/test.png"));
    REQUIRE(names_for_recognition[3]
        == path("test.archive/nested/test.png"));
    REQUIRE(names_for_conversion.size() == 1);
    REQUIRE(names_for_conversion[0] == path("nested/test.image"));
}
