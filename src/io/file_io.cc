#include <cassert>
#include <cstdio>
#include <stdexcept>
#include "compat/fopen.h"
#include "io/file_io.h"

using namespace au::io;

struct FileIO::Priv
{
    FILE *file;
};

void FileIO::seek(size_t offset)
{
    if (fseek(p->file, offset, SEEK_SET) != 0)
        throw std::runtime_error("Seeking beyond EOF");
}

void FileIO::skip(int offset)
{
    if (fseek(p->file, offset, SEEK_CUR) != 0)
        throw std::runtime_error("Seeking beyond EOF");
}

void FileIO::read(void *destination, size_t size)
{
    if (!size)
        return;
    assert(destination);
    if (fread(destination, 1, size, p->file) != size)
        throw std::runtime_error("Could not read full data");
}

void FileIO::write(const void *source, size_t size)
{
    if (!size)
        return;
    assert(source);
    if (fwrite(source, 1, size, p->file) != size)
        throw std::runtime_error("Could not write full data");
}

void FileIO::write_from_io(IO &source, size_t size)
{
    write(source.read(size));
}

size_t FileIO::tell() const
{
    return ftell(p->file);
}

size_t FileIO::size() const
{
    size_t old_pos = ftell(p->file);
    fseek(p->file, 0, SEEK_END);
    size_t size = ftell(p->file);
    fseek(p->file, old_pos, SEEK_SET);
    return size;
}

void FileIO::truncate(size_t new_size)
{
    if (new_size == size())
        return;
    //if (ftruncate(p->file, new_size) != 0)
        //throw std::runtime_error("Failed to truncate file");
    throw std::runtime_error("Not implemented");
}

FileIO::FileIO(const boost::filesystem::path &path, const FileMode mode)
    : p(new Priv())
{
    p->file = fopen(path, mode == FileMode::Write ? "w+b" : "r+b");
    if (!p->file)
        throw std::runtime_error("Could not open " + path.string());
}

FileIO::~FileIO()
{
    if (p->file)
        fclose(p->file);
}
