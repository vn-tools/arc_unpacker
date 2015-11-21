#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "fmt/decoder_util.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt;

namespace
{
    using path = boost::filesystem::path;

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

std::vector<std::string> TestArchiveDecoder::get_linked_formats() const
{
    return {"test/test"};
}

bool TestArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return true;
}

std::unique_ptr<ArchiveMeta>
    TestArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    auto entry = std::make_unique<ArchiveEntry>();
    entry->name = "infinity";
    meta->entries.push_back(std::move(entry));
    return meta;
}

std::unique_ptr<File> TestArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &, const ArchiveEntry &e) const
{
    arc_file.stream.seek(0);
    return std::make_unique<File>(e.name, arc_file.stream.read_to_eof());
}

TEST_CASE("Infinite recognition loops don't cause stack overflow", "[fmt_core]")
{
    auto registry = Registry::create_mock();
    registry->add_decoder(
        "test/test", []() { return std::make_unique<TestArchiveDecoder>(); });

    TestArchiveDecoder test_archive_decoder;
    File dummy_file("test.archive", "whatever"_b);

    std::vector<std::shared_ptr<File>> saved_files;
    const FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->stream.seek(0);
        saved_files.push_back(saved_file);
    });
    fmt::unpack_recursive(
        {}, test_archive_decoder, dummy_file, file_saver, *registry);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(boost::filesystem::basename(saved_files[0]->name) == "infinity");
    REQUIRE(saved_files[0]->stream.read_to_eof() == "whatever"_b);
}
