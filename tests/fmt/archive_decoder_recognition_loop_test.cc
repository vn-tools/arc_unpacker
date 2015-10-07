#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt;

namespace
{
    using path = boost::filesystem::path;

    class TestArchiveDecoder final : public ArchiveDecoder
    {
    public:
        TestArchiveDecoder();
        std::unique_ptr<ArchiveMeta> read_meta(File &) const override;
        std::unique_ptr<File> read_file(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    protected:
        bool is_recognized_internal(File &arc_file) const override;
    };
}

TestArchiveDecoder::TestArchiveDecoder()
{
    add_decoder(this);
}

bool TestArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return true;
}

std::unique_ptr<ArchiveMeta> TestArchiveDecoder::read_meta(File &arc_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    auto entry = std::make_unique<ArchiveEntry>();
    entry->name = "infinity";
    meta->entries.push_back(std::move(entry));
    return meta;
}

std::unique_ptr<File> TestArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &, const ArchiveEntry &e) const
{
    arc_file.io.seek(0);
    return std::make_unique<File>(e.name, arc_file.io.read_to_eof());
}

TEST_CASE("Infinite recognition loops don't cause stack overflow", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("whatever"_b);

    std::vector<std::shared_ptr<File>> saved_files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    test_archive_decoder.unpack(dummy_file, file_saver);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(boost::filesystem::basename(saved_files[0]->name) == "infinity");
    REQUIRE(saved_files[0]->io.read_to_eof() == "whatever"_b);
}
