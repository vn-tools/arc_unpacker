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
        bool is_recognized_impl(File &arc_file) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    };
}

bool TestArchiveDecoder::is_recognized_impl(File &arc_file) const
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
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    return std::make_unique<File>(entry->name, arc_file.io.read(entry->size));
}

TEST_CASE("Simple archive unpacks correctly", "[fmt_core]")
{
    const TestArchiveDecoder test_archive_decoder;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/file.txt"_b);
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(3);
    dummy_file.io.write("abc"_b);

    std::vector<std::shared_ptr<File>> saved_files;
    const FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    test_archive_decoder.unpack(dummy_file, file_saver);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("deeply/nested/file.txt"));
    REQUIRE(saved_files[0]->io.read_to_eof() == "abc"_b);
}
