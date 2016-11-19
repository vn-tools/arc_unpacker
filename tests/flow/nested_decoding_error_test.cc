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

bool TestFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("rgb");
}

std::unique_ptr<io::File> TestFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    throw std::logic_error("!");
}

std::vector<std::string> TestArchiveDecoder::get_linked_formats() const
{
    return {"test/test-image", "test/test-archive"};
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

TEST_CASE("Erroreus nested files still get unpacked", "[flow]")
{
    const auto registry = create_registry();
    const auto arc_content = make_archive(
        {
            tests::stub_file("undecoded.txt", "original"_b),
            tests::stub_file("erroreus.rgb", "original"_b),
        });

    io::File dummy_file("archive.arc", arc_content);
    const auto saved_files = tests::flow_unpack(*registry, true, dummy_file);

    REQUIRE(saved_files.size() == 2);
    tests::compare_paths(saved_files[0]->path, "archive.arc/erroreus.rgb");
    tests::compare_paths(saved_files[1]->path, "archive.arc/undecoded.txt");
    REQUIRE(saved_files[0]->stream.read_to_eof() == "original"_b);
    REQUIRE(saved_files[1]->stream.read_to_eof() == "original"_b);
}

TEST_CASE("Erroreus input files do not get copied", "[flow]")
{
    const auto registry = create_registry();

    auto dummy_file = tests::stub_file("erroreus.rgb", "original"_b);
    const auto saved_files = tests::flow_unpack(*registry, true, *dummy_file);

    REQUIRE(saved_files.size() == 0);
}

TEST_CASE("Erroreus nested archive entries do not copy archives", "[flow]")
{
    const auto registry = create_registry();
    const auto arc_content = make_archive(
        {
            tests::stub_file("erroreus.rgb", "original"_b),
            tests::stub_file(
                "nested.arc",
                make_archive(
                    {
                        tests::stub_file("undecoded.txt", "original"_b),
                        tests::stub_file("erroreus.rgb", "original"_b),
                    })),
        });

    io::File dummy_file("archive.arc", arc_content);
    const auto saved_files = tests::flow_unpack(*registry, true, dummy_file);

    REQUIRE(saved_files.size() == 3);
    tests::compare_paths(
        saved_files[0]->path, "archive.arc/nested.arc/erroreus.rgb");
    tests::compare_paths(
        saved_files[1]->path, "archive.arc/nested.arc/undecoded.txt");
    tests::compare_paths(saved_files[2]->path, "archive.arc/erroreus.rgb");
    REQUIRE(saved_files[0]->stream.read_to_eof() == "original"_b);
    REQUIRE(saved_files[1]->stream.read_to_eof() == "original"_b);
    REQUIRE(saved_files[2]->stream.read_to_eof() == "original"_b);
}
