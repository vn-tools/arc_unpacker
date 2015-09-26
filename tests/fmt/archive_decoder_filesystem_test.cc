#include <boost/filesystem.hpp>
#include "fmt/archive_decoder.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt;

namespace
{
    using path = boost::filesystem::path;
    using directory_iterator = boost::filesystem::directory_iterator;

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

static std::vector<std::shared_ptr<File>> unpack(
    File &file, ArchiveDecoder &decoder)
{
    std::vector<std::shared_ptr<File>> saved_files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> saved_file)
    {
        saved_file->io.seek(0);
        saved_files.push_back(saved_file);
    });
    decoder.unpack(file, file_saver, true);
    return saved_files;
}

TestArchiveDecoder::TestArchiveDecoder()
{
}

bool TestArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return true;
}

void TestArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &file_saver) const
{
    auto dir = path(arc_file.name).parent_path();
    for (directory_iterator it(dir); it != directory_iterator(); it++)
    {
        std::unique_ptr<File> output_file(new File);
        output_file->name = it->path().string();
        file_saver.save(std::move(output_file));
    }
}

TEST_CASE("Files get correct location", "[fmt_core]")
{
    auto input_path = path("./tests/fmt/archive_decoder_filesystem_test.cc");
    TestArchiveDecoder test_archive_decoder;
    File dummy_file(input_path, io::FileMode::Read);

    auto saved_files = unpack(dummy_file, test_archive_decoder);
    REQUIRE(saved_files.size() > 1);

    bool correct = false;
    for (auto &file : saved_files)
    {
        if (path(file->name) == input_path)
            correct = true;
    }
    REQUIRE(correct);
}
