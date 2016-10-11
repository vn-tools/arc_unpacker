// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "io/file_byte_stream.h"
#include <cstdio>
#include "algo/locale.h"
#include "err.h"

#if _WIN32
    #include <fcntl.h>
    #include <io.h>
    #include <sys/stat.h>
    #include <sys/types.h>
#endif

using namespace au;
using namespace au::io;

struct FileByteStream::Priv final
{
    #if _WIN32
        Priv(const path &path, FileMode mode) : path(path), mode(mode)
        {
            fd = _wopen(
                path.wstr().c_str(),
                (mode == FileMode::Write
                    ? (_O_RDWR | _O_CREAT | _O_TRUNC)
                    : _O_RDONLY)
                | _O_BINARY,
                _S_IREAD | _S_IWRITE);
            if (fd == -1)
                throw err::FileNotFoundError("Could not open " + path.str());
        }

        ~Priv()
        {
            _close(fd);
        }

        uoff_t tell()
        {
            return _telli64(fd);
        }

        void seek(const uoff_t offset, const int whence)
        {
            _lseeki64(fd, offset, whence);
        }

        void read(void *source, const size_t size)
        {
            const size_t ret = _read(fd, source, size);
            if (ret != size)
                throw err::EofError();
        }

        void write(const void *destination, const size_t size)
        {
            const size_t ret = _write(fd, destination, size);
            if (ret != size)
                throw err::IoError("Could not write full data");
        }

        int fd;
    #else
        Priv(const path &path, FileMode mode) : path(path), mode(mode)
        {
            fd = std::fopen(
                path.c_str(),
                mode == FileMode::Write ? "w+b" : "rb");
            if (!fd)
                throw err::FileNotFoundError("Could not open " + path.str());
        }

        ~Priv()
        {
            fclose(fd);
        }

        uoff_t tell()
        {
            return ftello(fd);
        }

        void seek(const uoff_t offset, const int whence)
        {
            const auto ret = fseeko(fd, offset, whence);
            if (ret != 0)
                throw err::EofError();
        }

        void read(void *source, const size_t size)
        {
            if (fread(source, 1, size, fd) != size)
                throw err::EofError();
        }

        void write(const void *destination, const size_t size)
        {
            if (fwrite(destination, 1, size, fd) != size)
                throw err::IoError("Could not write full data");
        }

        FILE *fd;
    #endif

    io::path path;
    FileMode mode;
};

FileByteStream::FileByteStream(const path &path, const FileMode mode)
    : p(new Priv(path, mode))
{
}

FileByteStream::~FileByteStream()
{
}

void FileByteStream::seek_impl(const uoff_t offset)
{
    if (offset > size())
        throw err::EofError();
    p->seek(offset, SEEK_SET);
}

void FileByteStream::read_impl(void *destination, const size_t size)
{
    // destination MUST exist and size MUST be at least 1
    p->read(destination, size);
}

void FileByteStream::write_impl(const void *source, const size_t size)
{
    // source MUST exist and size MUST be at least 1
    p->write(source, size);
}

uoff_t FileByteStream::pos() const
{
    return p->tell();
}

uoff_t FileByteStream::size() const
{
    const auto old_pos = p->tell();
    p->seek(0, SEEK_END);
    const auto size = p->tell();
    p->seek(old_pos, SEEK_SET);
    return size;
}

void FileByteStream::resize_impl(const uoff_t new_size)
{
    if (new_size == size())
        return;
    throw err::NotSupportedError("Truncating real files is not implemented");
}

std::unique_ptr<io::BaseByteStream> FileByteStream::clone() const
{
    auto ret = std::make_unique<FileByteStream>(p->path, p->mode);
    ret->seek(pos());
    return std::move(ret);
}
