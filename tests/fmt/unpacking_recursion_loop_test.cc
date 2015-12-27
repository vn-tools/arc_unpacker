#include "fmt/base_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/file_support.h"
#include "test_support/flow_support.h"

using namespace au;
using namespace au::fmt;

namespace
{
    class TestArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        std::vector<std::string> get_linked_formats() const override;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;
    };
}

std::vector<std::string> TestArchiveDecoder::get_linked_formats() const
{
    return {"test/test"};
}

bool TestArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return true;
}

std::unique_ptr<ArchiveMeta> TestArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    auto entry = std::make_unique<ArchiveEntry>();
    entry->path = "infinity";
    meta->entries.push_back(std::move(entry));
    return meta;
}

std::unique_ptr<io::File> TestArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const ArchiveMeta &,
    const ArchiveEntry &e) const
{
    return std::make_unique<io::File>(
        e.path, input_file.stream.seek(0).read_to_eof());
}

TEST_CASE("Infinite recognition loops don't cause stack overflow", "[fmt_core]")
{
    auto registry = Registry::create_mock();
    registry->add_decoder(
        "test/test", []() { return std::make_shared<TestArchiveDecoder>(); });

    TestArchiveDecoder test_archive_decoder;
    io::File dummy_file("test.archive", "whatever"_b);

    const auto saved_files = tests::flow_unpack(*registry, true, dummy_file);
    REQUIRE(saved_files.size() == 1);
    REQUIRE(saved_files[0]->path.name() == "infinity");
    REQUIRE(saved_files[0]->stream.read_to_eof() == "whatever"_b);
}
