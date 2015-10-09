#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt;

namespace
{
    using path = boost::filesystem::path;
    using directory_iterator = boost::filesystem::directory_iterator;

    class TestArchiveDecoder final : public ArchiveDecoder
    {
    public:
        TestArchiveDecoder();
    protected:
        bool is_recognized_impl(File &arc_file) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    };
}

static std::vector<std::shared_ptr<File>> unpack(
    File &file, ArchiveDecoder &decoder)
{
    std::vector<std::shared_ptr<File>> saved_files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    decoder.unpack(file, file_saver);
    return saved_files;
}

TestArchiveDecoder::TestArchiveDecoder()
{
}

bool TestArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return true;
}

std::unique_ptr<ArchiveMeta>
    TestArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(0);
    auto dir = path(arc_file.name).parent_path();
    auto meta = std::make_unique<ArchiveMeta>();
    for (directory_iterator it(dir); it != directory_iterator(); it++)
    {
        auto entry = std::make_unique<ArchiveEntry>();
        entry->name = it->path().string();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> TestArchiveDecoder::read_file_impl(
    File &, const ArchiveMeta &, const ArchiveEntry &e) const
{
    return std::make_unique<File>(e.name, ""_b);
}

TEST_CASE("Files get correct location", "[fmt_core]")
{
    auto input_path = path("./tests/fmt/archive_decoder_filesystem_test.cc");
    TestArchiveDecoder test_archive_decoder;
    File dummy_file(input_path, io::FileMode::Read);

    auto saved_files = unpack(dummy_file, test_archive_decoder);
    REQUIRE(saved_files.size() > 1);

    bool correct = false;
    for (auto &file : saved_files)
    {
        if (path(file->name) == input_path)
            correct = true;
    }
    REQUIRE(correct);
}
