#include "fmt/archive_decoder.h"
#include "fmt/file_decoder.h"
#include "fmt/decoder_util.h"
#include "io/memory_stream.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt;

namespace
{
    class TestFileDecoder final : public FileDecoder
    {
    public:
        std::function<void(io::File &)> recognition_callback;
        std::function<void(io::File &)> conversion_callback;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<io::File> decode_impl(
            io::File &input_file) const override;
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
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;
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
    io::MemoryStream stream;
    stream.write(name);
    stream.write_u8(0);
    stream.write_u32_le(content.size());
    stream.write(content);
    stream.seek(0);
    return stream.read_to_eof();
}

std::vector<std::string> TestArchiveDecoder::get_linked_formats() const
{
    return {"test/test-file", "test/test-archive"};
}

bool TestFileDecoder::is_recognized_impl(io::File &input_file) const
{
    if (recognition_callback)
        recognition_callback(input_file);
    return input_file.name.str().find("image") != std::string::npos;
}

std::unique_ptr<io::File> TestFileDecoder::decode_impl(
    io::File &input_file) const
{
    if (conversion_callback)
        conversion_callback(input_file);
    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("image"_b);
    output_file->name = input_file.name;
    output_file->name.change_extension("png");
    return output_file;
}

bool TestArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.name.str().find("archive") != std::string::npos;
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
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->name, data);
}

TEST_CASE("Recursive unpacking with nested files", "[fmt_core]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;
    io::File dummy_file("archive.arc", serialize_file("image.rgb", ""_b));

    std::vector<std::shared_ptr<io::File>> saved_files;
    const FileSaverCallback saver([&](std::shared_ptr<io::File> saved_file)
    {
        saved_file->stream.seek(0);
        saved_files.push_back(saved_file);
    });
    fmt::unpack_recursive({}, archive_decoder, dummy_file, saver, *registry);
    REQUIRE(saved_files.size() == 1);
    REQUIRE(saved_files[0]->name == io::path("image.png"));
    REQUIRE(saved_files[0]->stream.read_to_eof() == "image"_b);
}

TEST_CASE("Recursive unpacking with nested archives", "[fmt_core]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;

    io::MemoryStream nested_arc_stream;
    nested_arc_stream.write(serialize_file("nested/text.txt", "text"_b));
    nested_arc_stream.write(serialize_file("nested/image.rgb", "discard"_b));
    nested_arc_stream.seek(0);
    const auto nested_arc_content = nested_arc_stream.read_to_eof();

    io::File dummy_file(
        "archive.arc", serialize_file("archive.arc", nested_arc_content));

    std::vector<std::shared_ptr<io::File>> saved_files;
    const FileSaverCallback saver([&](std::shared_ptr<io::File> saved_file)
    {
        saved_file->stream.seek(0);
        saved_files.push_back(saved_file);
    });
    fmt::unpack_recursive({}, archive_decoder, dummy_file, saver, *registry);

    REQUIRE(saved_files.size() == 2);
    REQUIRE(saved_files[0]->name == io::path("archive.arc/nested/text.txt"));
    REQUIRE(saved_files[1]->name == io::path("archive.arc/nested/image.png"));
    REQUIRE(saved_files[0]->stream.read_to_eof() == "text"_b);
    REQUIRE(saved_files[1]->stream.read_to_eof() == "image"_b);
}

TEST_CASE(
    "Non-recursive unpacking doesn't execute child decoders", "[fmt_core]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;

    io::MemoryStream nested_arc_stream;
    nested_arc_stream.write(serialize_file("nested/text", "text"_b));
    nested_arc_stream.write(serialize_file("nested/image", "keep"_b));
    nested_arc_stream.seek(0);
    const auto nested_arc_content = nested_arc_stream.read_to_eof();

    io::File dummy_file(
        "archive.arc", serialize_file("archive.arc", nested_arc_content));

    std::vector<std::shared_ptr<io::File>> saved_files;
    const FileSaverCallback saver([&](std::shared_ptr<io::File> saved_file)
    {
        saved_file->stream.seek(0);
        saved_files.push_back(saved_file);
    });
    fmt::unpack_non_recursive({}, archive_decoder, dummy_file, saver);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(saved_files[0]->name == io::path("archive.arc"));
    REQUIRE(saved_files[0]->stream.read_to_eof() == nested_arc_content);
}

TEST_CASE(
    "Recursive unpacking passes correct paths to child decoders", "[fmt_core]")
{
    std::vector<io::path> names_for_recognition, names_for_conversion;
    auto registry = Registry::create_mock();
    registry->add_decoder(
        "test/test-archive",
        []() { return std::make_unique<TestArchiveDecoder>(); });
    registry->add_decoder(
        "test/test-file",
        [&]()
        {
            auto decoder = std::make_unique<TestFileDecoder>();
            decoder->recognition_callback = [&](io::File &f)
                { names_for_recognition.push_back(f.name); };
            decoder->conversion_callback = [&](io::File &f)
                { names_for_conversion.push_back(f.name); };
            return decoder;
        });

    TestArchiveDecoder archive_decoder;
    io::MemoryStream nested_arc_stream;
    nested_arc_stream.write(serialize_file("nested/test.image", ""_b));
    nested_arc_stream.seek(0);
    const auto nested_arc_content = nested_arc_stream.read_to_eof();

    io::File dummy_file(
        "fs/archive", serialize_file("archive.arc", nested_arc_content));

    const FileSaverCallback saver([](std::shared_ptr<io::File>) { });
    fmt::unpack_recursive({}, archive_decoder, dummy_file, saver, *registry);

    REQUIRE(names_for_recognition.size() == 3);
    REQUIRE(names_for_recognition[0] == io::path("archive.arc"));
    REQUIRE(names_for_recognition[1]
        == io::path("archive.arc/nested/test.image"));
    REQUIRE(names_for_recognition[2]
        == io::path("archive.arc/nested/test.png"));
    REQUIRE(names_for_conversion.size() == 1);
    REQUIRE(names_for_conversion[0]
        == io::path("archive.arc/nested/test.image"));
}
