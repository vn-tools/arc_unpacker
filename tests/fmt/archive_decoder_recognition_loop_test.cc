#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
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

    TestArchiveDecoder::TestArchiveDecoder()
    {
        add_decoder(this);
    }
}

bool TestArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return true;
}

void TestArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &file_saver) const
{
    std::unique_ptr<File> output_file(new File);
    output_file->name = "infinity";
    output_file->io.write_from_io(arc_file.io);
    file_saver.save(std::move(output_file));
}

TEST_CASE("Infinite recognition loops don't cause stack overflow", "[fmt_core]")
{
    TestArchiveDecoder test_archive_decoder;
    File dummy_file;
    dummy_file.name = "test.archive";
    dummy_file.io.write("whatever"_b);

    std::vector<std::shared_ptr<File>> saved_files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    test_archive_decoder.unpack(dummy_file, file_saver);

    REQUIRE(saved_files.size() == 1);
    REQUIRE(boost::filesystem::basename(saved_files[0]->name) == "infinity");
    REQUIRE(saved_files[0]->io.read_to_eof() == "whatever"_b);
}
