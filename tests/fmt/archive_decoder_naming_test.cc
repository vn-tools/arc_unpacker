#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "fmt/file_decoder.h"
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
    protected:
        bool is_recognized_internal(File &arc_file) const override;
        void unpack_internal(
            File &arc_file, FileSaver &file_saver) const override;
    };
}

TestArchiveDecoder::TestArchiveDecoder()
{
    add_decoder(this);
}

bool TestArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("archive");
}

void TestArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &file_saver) const
{
    while (!arc_file.io.eof())
    {
        std::unique_ptr<File> output_file(new File);
        output_file->name = arc_file.io.read_to_zero().str();
        size_t output_file_size = arc_file.io.read_u32_le();
        output_file->io.write(arc_file.io.read(output_file_size));
        file_saver.save(std::move(output_file));
    }
}

TEST_CASE("Simple archive unpacks correctly", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("deeply/nested/file.txt"_b);
    dummy_file.io.write_u8(0);
    dummy_file.io.write_u32_le(3);
    dummy_file.io.write("abc"_b);

    std::vector<std::shared_ptr<File>> saved_files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    test_archive_decoder.unpack(dummy_file, file_saver, true);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(path(saved_files[0]->name) == path("deeply/nested/file.txt"));
    REQUIRE(saved_files[0]->io.read_to_eof() == "abc"_b);
}
