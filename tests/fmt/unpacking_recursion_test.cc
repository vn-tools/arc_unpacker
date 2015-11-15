#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "fmt/file_decoder.h"
#include "fmt/decoder_util.h"
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
        bool is_recognized_impl(File &file) const override;
        std::unique_ptr<File> decode_impl(File &file) const override;
    };

    struct ArchiveEntryImpl final : ArchiveEntry
    {
        size_t offset;
        size_t size;
    };

    class TestArchiveDecoder final : public ArchiveDecoder
    {
    public:
        std::vector<std::string> get_linked_formats() const override;
    protected:
        bool is_recognized_impl(File &arc_file) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    };
}

static std::unique_ptr<Registry> create_registry()
{
    auto registry = Registry::create_mock();
    registry->add_decoder(
        "test/test-archive",
        []() { return std::make_unique<TestArchiveDecoder>(); });
    registry->add_decoder(
        "test/test-file",
        []() { return std::make_unique<TestFileDecoder>(); });
    return registry;
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

std::vector<std::string> TestArchiveDecoder::get_linked_formats() const
{
    return {"test/test-file", "test/test-archive"};
}

bool TestFileDecoder::is_recognized_impl(File &file) const
{
    if (recognition_callback)
        recognition_callback(file);
    return file.name.find("image") != std::string::npos;
}

std::unique_ptr<File> TestFileDecoder::decode_impl(File &file) const
{
    if (conversion_callback)
        conversion_callback(file);
    auto output_file = std::make_unique<File>();
    output_file->io.write("image"_b);
    output_file->name = file.name;
    output_file->change_extension("png");
    return output_file;
}

bool TestArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.name.find("archive") != std::string::npos;
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

TEST_CASE("Recursive unpacking with nested files", "[fmt_core]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;
    File dummy_file("archive.arc", serialize_file("image.rgb", ""_b));

    std::vector<std::shared_ptr<File>> saved_files;
    const FileSaverCallback saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    fmt::unpack_recursive({}, archive_decoder, dummy_file, saver, *registry);
    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("image.png"));
    REQUIRE(saved_files[0]->io.read_to_eof() == "image"_b);
}

TEST_CASE("Recursive unpacking with nested archives", "[fmt_core]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;

    io::BufferedIO nested_archive_io;
    nested_archive_io.write(serialize_file("nested/text.txt", "text"_b));
    nested_archive_io.write(serialize_file("nested/image.rgb", "discard"_b));
    nested_archive_io.seek(0);
    const auto nested_archive_content = nested_archive_io.read_to_eof();

    File dummy_file(
        "archive.arc", serialize_file("archive.arc", nested_archive_content));

    std::vector<std::shared_ptr<File>> saved_files;
    const FileSaverCallback saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    fmt::unpack_recursive({}, archive_decoder, dummy_file, saver, *registry);

    REQUIRE(saved_files.size() == 2);
    REQUIRE(path(saved_files[0]->name) == path("archive.arc/nested/text.txt"));
    REQUIRE(path(saved_files[1]->name) == path("archive.arc/nested/image.png"));
    REQUIRE(saved_files[0]->io.read_to_eof() == "text"_b);
    REQUIRE(saved_files[1]->io.read_to_eof() == "image"_b);
}

TEST_CASE(
    "Non-recursive unpacking doesn't execute child decoders", "[fmt_core]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;

    io::BufferedIO nested_archive_io;
    nested_archive_io.write(serialize_file("nested/text", "text"_b));
    nested_archive_io.write(serialize_file("nested/image", "keep"_b));
    nested_archive_io.seek(0);
    const auto nested_archive_content = nested_archive_io.read_to_eof();

    File dummy_file(
        "archive.arc", serialize_file("archive.arc", nested_archive_content));

    std::vector<std::shared_ptr<File>> saved_files;
    const FileSaverCallback saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    fmt::unpack_non_recursive({}, archive_decoder, dummy_file, saver);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("archive.arc"));
    REQUIRE(saved_files[0]->io.read_to_eof() == nested_archive_content);
}

TEST_CASE(
    "Recursive unpacking passes correct paths to child decoders", "[fmt_core]")
{
    std::vector<path> names_for_recognition, names_for_conversion;
    auto registry = Registry::create_mock();
    registry->add_decoder(
        "test/test-archive",
        []() { return std::make_unique<TestArchiveDecoder>(); });
    registry->add_decoder(
        "test/test-file",
        [&]()
        {
            auto decoder = std::make_unique<TestFileDecoder>();
            decoder->recognition_callback = [&](File &f)
                { names_for_recognition.push_back(path(f.name)); };
            decoder->conversion_callback = [&](File &f)
                { names_for_conversion.push_back(path(f.name)); };
            return decoder;
        });

    TestArchiveDecoder archive_decoder;
    io::BufferedIO nested_archive_io;
    nested_archive_io.write(serialize_file("nested/test.image", ""_b));
    nested_archive_io.seek(0);
    const auto nested_archive_content = nested_archive_io.read_to_eof();

    File dummy_file(
        "fs/archive", serialize_file("archive.arc", nested_archive_content));

    const FileSaverCallback saver([](std::shared_ptr<File>) { });
    fmt::unpack_recursive({}, archive_decoder, dummy_file, saver, *registry);

    REQUIRE(names_for_recognition.size() == 3);
    REQUIRE(names_for_recognition[0] == path("archive.arc"));
    REQUIRE(names_for_recognition[1] == path("archive.arc/nested/test.image"));
    REQUIRE(names_for_recognition[2] == path("archive.arc/nested/test.png"));
    REQUIRE(names_for_conversion.size() == 1);
    REQUIRE(names_for_conversion[0] == path("archive.arc/nested/test.image"));
}
