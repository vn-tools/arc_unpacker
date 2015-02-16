#include <cassert>
#include <cstdio>
#include <stdexcept>
#include "compat/open.h"
#include "file_io.h"

struct FileIO::Internals
{
    FILE *file;
};

void FileIO::seek(size_t offset)
{
    if (fseek(internals->file, offset, SEEK_SET) != 0)
        throw std::runtime_error("Failed to seek");
}

void FileIO::skip(int offset)
{
    if (fseek(internals->file, offset, SEEK_CUR) != 0)
        throw std::runtime_error("Failed to seek");
}

void FileIO::read(void *destination, size_t length)
{
    assert(destination != nullptr);
    if (fread(destination, 1, length, internals->file) != length)
        throw std::runtime_error("Failed to read full data");
}

void FileIO::write(const void *source, size_t length)
{
    assert(source != nullptr);
    if (fwrite(source, 1, length, internals->file) != length)
        throw std::runtime_error("Failed to write full data");
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

void FileIO::truncate(size_t)
{
    //if (ftruncate(internals->file, new_size) != 0)
        //throw std::runtime_error("Failed to truncate file");
    throw std::runtime_error("Not implemented");
}

FileIO::FileIO(std::string path, std::string read_mode)
    : internals(new FileIO::Internals)
{
    internals->file = compat_open(path.c_str(), read_mode.c_str());
    if (internals->file == nullptr)
        throw std::runtime_error("Can't open file " + path);
}

FileIO::~FileIO()
{
    if (internals != nullptr)
        fclose(internals->file);
}
