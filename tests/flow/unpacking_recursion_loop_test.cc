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
#include "test_support/catch.h"
#include "test_support/file_support.h"
#include "test_support/flow_support.h"

using namespace au;
using namespace au::dec;

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

TEST_CASE("Infinite recognition loops don't cause stack overflow", "[flow]")
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
