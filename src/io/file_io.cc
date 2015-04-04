#include <cstdio>
#include <stdexcept>
#include "compat/fopen.h"
#include "io/file_io.h"

struct FileIO::Internals
{
    FILE *file;
};

void FileIO::seek(size_t offset)
{
    if (fseek(internals->file, offset, SEEK_SET) != 0)
        throw std::runtime_error("Seeking beyond EOF");
}

void FileIO::skip(int offset)
{
    if (fseek(internals->file, offset, SEEK_CUR) != 0)
        throw std::runtime_error("Seeking beyond EOF");
}

void FileIO::read(void *destination, size_t length)
{
    if (fread(destination, 1, length, internals->file) != length)
        throw std::runtime_error("Could not read full data");
}

void FileIO::write(const void *source, size_t length)
{
    if (fwrite(source, 1, length, internals->file) != length)
        throw std::runtime_error("Could not write full data");
}

void FileIO::write_from_io(IO &source, size_t length)
{
    // TODO improvement: use static buffer instead of such allocation
    std::unique_ptr<char> buffer(new char[length]);
    source.read(buffer.get(), length);
    write(buffer.get(), length);
}

size_t FileIO::tell() const
{
    return ftell(internals->file);
}

size_t FileIO::size() const
{
    size_t old_pos = ftell(internals->file);
    fseek(internals->file, 0, SEEK_END);
    size_t size = ftell(internals->file);
    fseek(internals->file, old_pos, SEEK_SET);
    return size;
}

void FileIO::truncate(size_t new_size)
{
    if (new_size == size())
        return;
    //if (ftruncate(internals->file, new_size) != 0)
        //throw std::runtime_error("Failed to truncate file");
    throw std::runtime_error("Not implemented");
}

FileIO::FileIO(const boost::filesystem::path &path, const FileIOMode mode)
    : internals(new Internals())
{
    internals->file = fopen(path, mode == FileIOMode::Write ? "w+b" : "r+b");
    if (internals->file == nullptr)
        throw std::runtime_error("Could not open " + path.string());
}

FileIO::~FileIO()
{
    if (internals->file != nullptr)
        fclose(internals->file);
}
