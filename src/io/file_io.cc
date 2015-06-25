#include <cstdio>
#include <stdexcept>
#include "compat/fopen.h"
#include "io/file_io.h"

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

void FileIO::read(void *destination, size_t length)
{
    if (fread(destination, 1, length, p->file) != length)
        throw std::runtime_error("Could not read full data");
}

void FileIO::write(const void *source, size_t length)
{
    if (fwrite(source, 1, length, p->file) != length)
        throw std::runtime_error("Could not write full data");
}

void FileIO::write_from_io(IO &source, size_t length)
{
    // TODO improvement: use static buffer instead of such allocation
    std::unique_ptr<char[]> buffer(new char[length]);
    source.read(buffer.get(), length);
    write(buffer.get(), length);
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

FileIO::FileIO(const boost::filesystem::path &path, const FileIOMode mode)
    : p(new Priv())
{
    p->file = fopen(path, mode == FileIOMode::Write ? "w+b" : "r+b");
    if (p->file == nullptr)
        throw std::runtime_error("Could not open " + path.string());
}

FileIO::~FileIO()
{
    if (p->file != nullptr)
        fclose(p->file);
}
