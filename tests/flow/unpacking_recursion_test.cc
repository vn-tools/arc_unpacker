// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/base_archive_decoder.h"
#include "dec/base_file_decoder.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/common.h"
#include "test_support/file_support.h"
#include "test_support/flow_support.h"

using namespace au;
using namespace au::dec;

namespace
{
    class TestFileDecoder final : public BaseFileDecoder
    {
    public:
        std::function<void(io::File &)> recognition_callback;
        std::function<void(io::File &)> conversion_callback;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<io::File> decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

    class TestArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        std::vector<std::string> get_linked_formats() const override;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger, io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
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
        []() { return std::make_shared<TestArchiveDecoder>(); });
    registry->add_decoder(
        "test/test-image",
        []() { return std::make_shared<TestFileDecoder>(); });
    return registry;
}

static bstr make_archive(
    std::initializer_list<std::shared_ptr<io::File>> input_files)
{
    io::MemoryByteStream tmp_stream;
    for (auto &input_file : input_files)
    {
        const auto content = input_file->stream.seek(0).read_to_eof();
        tmp_stream.write(input_file->path.str());
        tmp_stream.write<u8>(0);
        tmp_stream.write_le<u32>(content.size());
        tmp_stream.write(content);
    }
    return tmp_stream.seek(0).read_to_eof();
}

std::vector<std::string> TestArchiveDecoder::get_linked_formats() const
{
    return {"test/test-image", "test/test-archive"};
}

bool TestFileDecoder::is_recognized_impl(io::File &input_file) const
{
    if (recognition_callback)
        recognition_callback(input_file);
    return input_file.path.has_extension("rgb");
}

std::unique_ptr<io::File> TestFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    if (conversion_callback)
        conversion_callback(input_file);
    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("decoded_image"_b);
    output_file->path = input_file.path;
    output_file->path.change_extension("png");
    return output_file;
}

bool TestArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("arc");
}

std::unique_ptr<ArchiveMeta> TestArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = std::make_unique<ArchiveMeta>();
    while (input_file.stream.left())
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = input_file.stream.read_to_zero().str();
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> TestArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const ArchiveMeta &,
    const ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

TEST_CASE("Recursive unpacking with nested files", "[flow]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;

    const auto arc_content = make_archive(
        {
            tests::stub_file("image.rgb", "discard"_b),
        });

    io::File dummy_file("archive.arc", arc_content);

    const auto saved_files = tests::flow_unpack(*registry, true, dummy_file);
    REQUIRE(saved_files.size() == 1);
    tests::compare_paths(saved_files[0]->path, "archive.arc/image.png");
    REQUIRE(saved_files[0]->stream.read_to_eof() == "decoded_image"_b);
}

TEST_CASE("Recursive unpacking with nested archives", "[flow]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;

    const auto inner_arc_content = make_archive(
        {
            tests::stub_file("nested/image.rgb", "discard"_b),
            tests::stub_file("nested/text.txt", "text"_b),
        });

    const auto outer_arc_content = make_archive(
        {
            tests::stub_file("inner.arc", inner_arc_content),
        });

    io::File dummy_file("outer.arc", outer_arc_content);

    const auto saved_files = tests::flow_unpack(*registry, true, dummy_file);
    REQUIRE(saved_files.size() == 2);
    tests::compare_paths(
        saved_files[0]->path, "outer.arc/inner.arc/nested/text.txt");
    tests::compare_paths(
        saved_files[1]->path, "outer.arc/inner.arc/nested/image.png");
    REQUIRE(saved_files[0]->stream.read_to_eof() == "text"_b);
    REQUIRE(saved_files[1]->stream.read_to_eof() == "decoded_image"_b);
}

TEST_CASE(
    "Non-recursive unpacking doesn't execute child decoders", "[flow]")
{
    const auto registry = create_registry();

    const auto inner_arc_content = make_archive(
        {
            tests::stub_file("nested/unknown.txt", "text"_b),
            tests::stub_file("nested/image.rgb", "keep"_b),
        });

    const auto outer_arc_content = make_archive(
        {
            tests::stub_file("inner.arc", inner_arc_content),
        });

    io::File dummy_file("outer.arc", outer_arc_content);

    const auto saved_files = tests::flow_unpack(*registry, false, dummy_file);

    REQUIRE(saved_files.size() == 1);
    tests::compare_paths(saved_files[0]->path, "outer.arc/inner.arc");
    REQUIRE(saved_files[0]->stream.read_to_eof() == inner_arc_content);
}

TEST_CASE(
    "Recursive unpacking passes correct paths to child decoders", "[flow]")
{
    std::vector<io::path> paths_for_recognition, paths_for_conversion;
    auto registry = Registry::create_mock();
    registry->add_decoder(
        "test/test-archive",
        []() { return std::make_shared<TestArchiveDecoder>(); });
    registry->add_decoder(
        "test/test-image",
        [&]()
        {
            auto decoder = std::make_shared<TestFileDecoder>();
            decoder->recognition_callback = [&](io::File &f)
                { paths_for_recognition.push_back(f.path); };
            decoder->conversion_callback = [&](io::File &f)
                { paths_for_conversion.push_back(f.path); };
            return decoder;
        });

    const auto inner_arc_content = make_archive(
        {
            tests::stub_file("nested/test.rgb", ""_b),
        });

    const auto outer_arc_content = make_archive(
        {
            tests::stub_file("inner.arc", inner_arc_content),
        });

    io::File dummy_file("outer.arc", outer_arc_content);

    const auto saved_files = tests::flow_unpack(*registry, true, dummy_file);
    REQUIRE(paths_for_recognition.size() == 5);
    tests::compare_paths(paths_for_recognition[0], "outer.arc");
    tests::compare_paths(
        paths_for_recognition[1], "outer.arc/inner.arc");
    tests::compare_paths(
        paths_for_recognition[2], "outer.arc/inner.arc/nested/test.rgb");
    tests::compare_paths(
        paths_for_recognition[3], "outer.arc/inner.arc/nested/test.rgb");
    tests::compare_paths(
        paths_for_recognition[4], "outer.arc/inner.arc/nested/test.png");
    REQUIRE(paths_for_conversion.size() == 1);
    tests::compare_paths(
        paths_for_conversion[0], "outer.arc/inner.arc/nested/test.rgb");
    REQUIRE(saved_files.size() == 1);
    tests::compare_paths(
        saved_files[0]->path, "outer.arc/inner.arc/nested/test.png");
}
