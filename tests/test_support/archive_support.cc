#include "arg_parser.h"
#include "file.h"
#include "io/file_io.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hpp"

std::vector<std::shared_ptr<File>> unpack_to_memory(
    const boost::filesystem::path &input_path, Archive &archive)
{
    ArgParser arg_parser;
    File file(input_path, FileIOMode::Read);
    std::vector<std::shared_ptr<File>> files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> file)
    {
        files.push_back(file);
    });
    archive.unpack(file, file_saver);
    return files;
}

void compare_files(
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
        for (size_t j = 0; j < expected_file->io.size(); j++)
            REQUIRE(expected_file->io.read_u8() == actual_file->io.read_u8());
    }
}
