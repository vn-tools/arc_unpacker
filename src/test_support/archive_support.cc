#include <cassert>
#include "arg_parser.h"
#include "file.h"
#include "file_io.h"
#include "test_support/archive_support.h"
#include "test_support/eassert.h"

std::unique_ptr<FileSaverMemory> unpack_to_memory(
    const boost::filesystem::path &input_path, Archive &archive)
{
    ArgParser arg_parser;
    File file(input_path, FileIOMode::Read);
    std::unique_ptr<FileSaverMemory> file_saver(new FileSaverMemory);
    archive.unpack(file, *file_saver);
    return file_saver;
}

void compare_files(
    const std::vector<File*> &expected_files,
    std::unique_ptr<FileSaverMemory> file_saver)
{
    auto actual_files = file_saver->get_saved();
    eassert(actual_files.size() == expected_files.size());
    for (size_t i = 0; i < expected_files.size(); i ++)
    {
        auto &expected_file = expected_files[i];
        auto &actual_file = actual_files[i];
        eassert(expected_file->name == actual_file->name);
        eassert(expected_file->io.size() == actual_file->io.size());
        expected_file->io.seek(0);
        actual_file->io.seek(0);
        for (size_t j = 0; j < expected_file->io.size(); j ++)
            eassert(expected_file->io.read_u8() == actual_file->io.read_u8());
    }
}
