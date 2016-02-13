#include "io/file_stream.h"
#include <cstdio>
#include "algo/locale.h"
#include "err.h"

using namespace au;
using namespace au::io;

static FILE *utf8_fopen(const path &path, const char *mode)
{
    #ifdef _WIN32
        const auto cmode = algo::utf8_to_utf16(std::string(mode)).str();
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
    io::path path;
    FileMode mode;
};

FileStream::FileStream(const path &path, const FileMode mode)
    : p(new Priv())
{
    p->file = utf8_fopen(path, mode == FileMode::Write ? "w+b" : "r+b");
    p->path = path;
    p->mode = mode;
    if (!p->file)
        throw err::FileNotFoundError("Could not open " + path.str());
}

FileStream::~FileStream()
{
    if (p->file)
        fclose(p->file);
}

void FileStream::seek_impl(const size_t offset)
{
    if (offset > size() || fseek(p->file, offset, SEEK_SET) != 0)
        throw err::EofError();
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

size_t FileStream::pos() const
{
    return ftell(p->file);
}

size_t FileStream::size() const
{
    const auto old_pos = ftell(p->file);
    fseek(p->file, 0, SEEK_END);
    const auto size = ftell(p->file);
    fseek(p->file, old_pos, SEEK_SET);
    return size;
}

void FileStream::resize_impl(const size_t new_size)
{
    if (new_size == size())
        return;
    throw err::NotSupportedError("Truncating real files is not implemented");
}

std::unique_ptr<io::BaseByteStream> FileStream::clone() const
{
    auto ret = std::make_unique<FileStream>(p->path, p->mode);
    ret->seek(pos());
    return std::move(ret);
}
