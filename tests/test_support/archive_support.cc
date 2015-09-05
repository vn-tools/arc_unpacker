#include "file.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"

using namespace au;

std::vector<std::shared_ptr<File>> tests::unpack_to_memory(
    const boost::filesystem::path &input_path, fmt::Archive &archive)
{
    File file(input_path, io::FileMode::Read);
    return unpack_to_memory(file, archive);
}

std::vector<std::shared_ptr<File>> tests::unpack_to_memory(
    File &file, fmt::Archive &archive)
{
    std::vector<std::shared_ptr<File>> files;
    FileSaverCallback file_saver([&](std::shared_ptr<File> file)
    {
        files.push_back(file);
    });
    archive.unpack(file, file_saver, true);
    return files;
}
