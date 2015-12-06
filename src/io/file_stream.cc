#include "io/file_stream.h"
#include <cstdio>
#include "algo/locale.h"
#include "err.h"

using namespace au;
using namespace au::io;

static FILE *utf8_fopen(const path &path, const char *mode)
{
    #ifdef _WIN32
        auto cmode = algo::convert_locale(
            std::string(mode), "utf-8", "utf-16le").str();
        std::wstring widemode(
            reinterpret_cast<const wchar_t*>(cmode.c_str()),
            cmode.size() / 2);
        return _wfopen(path.wstr().c_str(), widemode.c_str());
    #else
        return std::fopen(path.c_str(), mode);
    #endif
}

struct FileStream::Priv final
{
    FILE *file;
};

FileStream::FileStream(const path &path, const FileMode mode)
    : p(new Priv())
{
    p->file = utf8_fopen(path, mode == FileMode::Write ? "w+b" : "r+b");
    if (!p->file)
        throw err::FileNotFoundError("Could not open " + path.str());
}

FileStream::~FileStream()
{
    if (p->file)
        fclose(p->file);
}

Stream &FileStream::seek(const size_t offset)
{
    if (offset > size() || fseek(p->file, offset, SEEK_SET) != 0)
        throw err::EofError();
    return *this;
}

Stream &FileStream::skip(const int offset)
{
    if (tell() + offset > size() || fseek(p->file, offset, SEEK_CUR) != 0)
        throw err::EofError();
    return *this;
}

void FileStream::read_impl(void *destination, const size_t size)
{
    // destination MUST exist and size MUST be at least 1
    if (fread(destination, 1, size, p->file) != size)
        throw err::EofError();
}

void FileStream::write_impl(const void *source, const size_t size)
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

Stream &FileStream::truncate(const size_t new_size)
{
    if (new_size == size())
        return *this;
    throw err::NotSupportedError("Truncating real files is not implemented");
}
