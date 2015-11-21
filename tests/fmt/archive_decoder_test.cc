#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "fmt/file_decoder.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt;

namespace
{
    using path = boost::filesystem::path;

    struct ArchiveEntryImpl final : ArchiveEntry
    {
        size_t offset;
        size_t size;
    };

    class TestArchiveDecoder final : public ArchiveDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;
    };
}

bool TestArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.has_extension("archive");
}

std::unique_ptr<ArchiveMeta>
    TestArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = std::make_unique<ArchiveMeta>();
    while (!input_file.stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = input_file.stream.read_to_zero().str();
        entry->size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.tell();
        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> TestArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->name, data);
}

TEST_CASE("Simple archive unpacks correctly", "[fmt_core]")
{
    const TestArchiveDecoder test_archive_decoder;
    io::File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.stream.write("deeply/nested/file.txt"_b);
    dummy_file.stream.write_u8(0);
    dummy_file.stream.write_u32_le(3);
    dummy_file.stream.write("abc"_b);

    std::vector<std::shared_ptr<io::File>> saved_files;
    const FileSaverCallback file_saver([&](std::shared_ptr<io::File> saved_file)
    {
        saved_file->stream.seek(0);
        saved_files.push_back(saved_file);
    });
    test_archive_decoder.unpack(dummy_file, file_saver);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("deeply/nested/file.txt"));
    REQUIRE(saved_files[0]->stream.read_to_eof() == "abc"_b);
}
