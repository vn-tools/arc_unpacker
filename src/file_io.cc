#include <boost/filesystem/fstream.hpp>
#include <cstdio>
#include <stdexcept>
#include "file_io.h"

struct FileIO::Internals
{
    boost::filesystem::basic_fstream<char> stream;

    Internals(const std::string &path, const FileIOMode mode)
    {
        std::ios_base::openmode flags;

        flags |= std::ios_base::in;
        if (mode == FileIOMode::Write)
        {
            flags |= std::ios_base::out;
            flags |= std::ios_base::trunc;
        }
        flags |= std::ios_base::binary;

        stream.open(boost::filesystem::path(path), flags);
        stream.exceptions(std::fstream::failbit | std::fstream::badbit);
    }
};

void FileIO::seek(size_t offset)
{
    internals->stream.seekg(offset, std::fstream::beg);
}

void FileIO::skip(int offset)
{
    internals->stream.seekg(offset, std::fstream::cur);
}

void FileIO::read(void *destination, size_t length)
{
    internals->stream.read(reinterpret_cast<char*>(destination), length);
}

void FileIO::write(const void *source, size_t length)
{
    internals->stream.write(reinterpret_cast<const char*>(source), length);
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
    return internals->stream.tellg();
}

size_t FileIO::size() const
{
    size_t old_pos = internals->stream.tellg();
    internals->stream.seekg(0, std::fstream::end);
    size_t size = internals->stream.tellg();
    internals->stream.seekg(old_pos, std::fstream::beg);
    return size;
}

void FileIO::truncate(size_t)
{
    //if (ftruncate(internals->file, new_size) != 0)
        //throw std::runtime_error("Failed to truncate file");
    throw std::runtime_error("Not implemented");
}

FileIO::FileIO(const std::string &path, const FileIOMode mode)
    : internals(new Internals(path, mode))
{
}

FileIO::~FileIO()
{
    internals->stream.close();
}
