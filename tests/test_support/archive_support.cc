#include "arg_parser.h"
#include "file.h"
#include "io/file_io.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"

using namespace au;

std::vector<std::shared_ptr<File>> au::tests::unpack_to_memory(
    const boost::filesystem::path &input_path, fmt::Archive &archive)
{
    ArgParser arg_parser;
    File file(input_path, io::FileMode::Read);
    std::vector<std::shared_ptr<File>> files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> file)
    {
        files.push_back(file);
    });
    archive.unpack(file, file_saver);
    return files;
}

void au::tests::compare_files(
    const std::vector<std::shared_ptr<File>> &expected_files,
    const std::vector<std::shared_ptr<File>> &actual_files)
{
    REQUIRE(actual_files.size() == expected_files.size());
    for (size_t i = 0; i < expected_files.size(); i++)
    {
        auto &expected_file = expected_files[i];
        auto &actual_file = actual_files[i];
        REQUIRE(expected_file->name == actual_file->name);
        REQUIRE(expected_file->io.size() == actual_file->io.size());
        expected_file->io.seek(0);
        actual_file->io.seek(0);
        auto expected_content = expected_file->io.read_to_eof();
        auto actual_content = actual_file->io.read_to_eof();
        INFO("Expected content: " << expected_content.str());
        INFO("Actual content: " << actual_content.str());
        REQUIRE(expected_content == actual_content);
    }
}
