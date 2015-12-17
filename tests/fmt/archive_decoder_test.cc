#include "fmt/archive_decoder.h"
#include "fmt/file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"

using namespace au;
using namespace au::fmt;

namespace
{
    struct ArchiveEntryImpl final : ArchiveEntry
    {
        size_t offset;
        size_t size;
    };

    class TestArchiveDecoder final : public ArchiveDecoder
    {
    public:
        TestArchiveDecoder(const NamingStrategy strategy);

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;

        NamingStrategy naming_strategy() const override;

    private:
        NamingStrategy strategy;
    };
}

TestArchiveDecoder::TestArchiveDecoder(const NamingStrategy strategy)
    : strategy(strategy)
{
}

IDecoder::NamingStrategy TestArchiveDecoder::naming_strategy() const
{
    return strategy;
}

bool TestArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("archive");
}

std::unique_ptr<ArchiveMeta>
    TestArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = std::make_unique<ArchiveMeta>();
    while (!input_file.stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero().str();
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
    return std::make_unique<io::File>(entry->path, data);
}

TEST_CASE("Simple archive unpacks correctly", "[fmt_core]")
{
    const TestArchiveDecoder test_archive_decoder(
        IDecoder::NamingStrategy::Child);
    io::File dummy_file;
    dummy_file.path = "test.archive";
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
    REQUIRE(saved_files[0]->path == io::path("deeply/nested/file.txt"));
    REQUIRE(saved_files[0]->stream.read_to_eof() == "abc"_b);
}

TEST_CASE("Archive files get proper fallback names", "[fmt_core]")
{
    SECTION("Child naming strategy")
    {
        const TestArchiveDecoder test_archive_decoder(
            IDecoder::NamingStrategy::Child);

        SECTION("Just one file")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 1);
            REQUIRE(saved_files[0]->path.name() == "unk");
        }

        SECTION("Multiple files")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 2);
            REQUIRE(saved_files[0]->path.str() == "unk_000");
            REQUIRE(saved_files[1]->path.str() == "unk_001");
        }

        SECTION("Mixed nameless and named files")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write("named"_b);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 3);
            REQUIRE(saved_files[0]->path.str() == "unk_000");
            REQUIRE(saved_files[1]->path.str() == "named");
            REQUIRE(saved_files[2]->path.str() == "unk_001");
        }
    }

    SECTION("Root naming strategy")
    {
        const TestArchiveDecoder test_archive_decoder(
            IDecoder::NamingStrategy::Root);

        SECTION("Just one file")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 1);
            REQUIRE(saved_files[0]->path.name() == "test.archive");
        }

        SECTION("Multiple files")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 2);
            REQUIRE(saved_files[0]->path.str() == "path/test.archive_000");
            REQUIRE(saved_files[1]->path.str() == "path/test.archive_001");
        }

        SECTION("Mixed nameless and named files")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write("named"_b);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 3);
            REQUIRE(saved_files[0]->path.str() == "path/test.archive_000");
            REQUIRE(saved_files[1]->path.str() == "named");
            REQUIRE(saved_files[2]->path.str() == "path/test.archive_001");
        }
    }

    SECTION("Sibling naming strategy")
    {
        const TestArchiveDecoder test_archive_decoder(
            IDecoder::NamingStrategy::Sibling);

        SECTION("Just one file")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 1);
            REQUIRE(saved_files[0]->path.name() == "test");
        }

        SECTION("Multiple files")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 2);
            REQUIRE(saved_files[0]->path.str() == "test_000");
            REQUIRE(saved_files[1]->path.str() == "test_001");
        }

        SECTION("Mixed nameless and named files")
        {
            io::File dummy_file;
            dummy_file.path = "path/test.archive";
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write("named"_b);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);
            dummy_file.stream.write_u8(0);
            dummy_file.stream.write_u32_le(0);

            auto saved_files = tests::unpack(test_archive_decoder, dummy_file);
            REQUIRE(saved_files.size() == 3);
            REQUIRE(saved_files[0]->path.str() == "test_000");
            REQUIRE(saved_files[1]->path.str() == "named");
            REQUIRE(saved_files[2]->path.str() == "test_001");
        }
    }
}
