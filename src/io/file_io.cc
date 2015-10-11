#include "io/file_io.h"
#include <cstdio>
#include "err.h"
#include "compat/fopen.h"

using namespace au::io;

struct FileIO::Priv final
{
    FILE *file;
};

FileIO::FileIO(const boost::filesystem::path &path, const FileMode mode)
    : p(new Priv())
{
    p->file = fopen(path, mode == FileMode::Write ? "w+b" : "r+b");
    if (!p->file)
        throw err::FileNotFoundError("Could not open " + path.string());
}

FileIO::~FileIO()
{
    if (p->file)
        fclose(p->file);
}

void FileIO::seek(size_t offset)
{
    if (fseek(p->file, offset, SEEK_SET) != 0)
        throw err::EofError();
}

void FileIO::skip(int offset)
{
    if (fseek(p->file, offset, SEEK_CUR) != 0)
        throw err::EofError();
}

void FileIO::read(void *destination, size_t size)
{
    if (!size)
        return;
    if (!destination)
        throw std::logic_error("Reading to nullptr");
    if (fread(destination, 1, size, p->file) != size)
        throw err::EofError();
}

void FileIO::write(const void *source, size_t size)
{
    if (!size)
        return;
    if (!source)
        throw std::logic_error("Writing from nullptr");
    if (fwrite(source, 1, size, p->file) != size)
        throw err::IoError("Could not write full data");
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
    throw err::NotSupportedError("Truncating real files is not implemented");
}
