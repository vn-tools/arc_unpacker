#include "io/file_stream.h"
#include <cstdio>
#include "err.h"
#include "compat/fopen.h"

using namespace au::io;

struct FileStream::Priv final
{
    FILE *file;
};

FileStream::FileStream(const boost::filesystem::path &path, const FileMode mode)
    : p(new Priv())
{
    p->file = fopen(path, mode == FileMode::Write ? "w+b" : "r+b");
    if (!p->file)
        throw err::FileNotFoundError("Could not open " + path.string());
}

FileStream::~FileStream()
{
    if (p->file)
        fclose(p->file);
}

Stream &FileStream::seek(size_t offset)
{
    if (fseek(p->file, offset, SEEK_SET) != 0)
        throw err::EofError();
    return *this;
}

Stream &FileStream::skip(int offset)
{
    if (tell() + offset > size() || fseek(p->file, offset, SEEK_CUR) != 0)
        throw err::EofError();
    return *this;
}

void FileStream::read_impl(void *destination, size_t size)
{
    // destination MUST exist and size MUST be at least 1
    if (fread(destination, 1, size, p->file) != size)
        throw err::EofError();
}

void FileStream::write_impl(const void *source, size_t size)
{
    // source MUST exist and size MUST be at least 1
    if (fwrite(source, 1, size, p->file) != size)
        throw err::IoError("Could not write full data");
}

size_t FileStream::tell() const
{
    return ftell(p->file);
}

size_t FileStream::size() const
{
    size_t old_pos = ftell(p->file);
    fseek(p->file, 0, SEEK_END);
    size_t size = ftell(p->file);
    fseek(p->file, old_pos, SEEK_SET);
    return size;
}

void FileStream::truncate(size_t new_size)
{
    if (new_size == size())
        return;
    throw err::NotSupportedError("Truncating real files is not implemented");
}
