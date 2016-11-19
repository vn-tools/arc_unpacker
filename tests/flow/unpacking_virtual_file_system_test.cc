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
#include "virtual_file_system.h"

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

std::vector<std::string> TestArchiveDecoder::get_linked_formats() const
{
    return {"test/test-image", "test/test-archive"};
}

bool TestFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("rgb");
}

std::unique_ptr<io::File> TestFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto output_file = std::make_unique<io::File>();
    auto other_file = VirtualFileSystem::get_by_stem("aside");
    if (other_file)
    {
        output_file->stream.write(other_file->stream.seek(0).read_to_eof());
        output_file->stream.write("_used"_b);
    }
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

TEST_CASE("Recursive unpacking with virtual file system lookups", "[flow]")
{
    const auto registry = create_registry();
    TestArchiveDecoder archive_decoder;

    const auto inner_arc_content = make_archive(
        {
            tests::stub_file("nested/image.rgb", "discard"_b),
            tests::stub_file("nested/aside.txt", "aside"_b),
        });

    const auto outer_arc_content = make_archive(
        {
            tests::stub_file("inner.arc", inner_arc_content),
        });

    io::File dummy_file("outer.arc", outer_arc_content);

    const auto saved_files = tests::flow_unpack(*registry, true, dummy_file);
    REQUIRE(saved_files.size() == 2);
    tests::compare_paths(
        saved_files[0]->path, "outer.arc/inner.arc/nested/aside.txt");
    tests::compare_paths(
        saved_files[1]->path, "outer.arc/inner.arc/nested/image.png");
    REQUIRE(saved_files[0]->stream.read_to_eof().str() == "aside");
    REQUIRE(saved_files[1]->stream.read_to_eof().str() == "aside_used");
}
