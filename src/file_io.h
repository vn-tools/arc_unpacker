#ifndef FILE_IO
#define FILE_IO
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "io.h"

enum FileIOMode
{
    Read = 1,
    Write = 2,
};

class FileIO final : public IO
{
public:
    FileIO(const boost::filesystem::path &path, const FileIOMode mode);
    ~FileIO();

    virtual size_t size() const override;
    virtual size_t tell() const override;
    virtual void seek(size_t offset) override;
    virtual void skip(int offset) override;
    virtual void truncate(size_t new_size) override;

    using IO::read;
    virtual void read(void *destination, size_t length) override;

    using IO::write;
    using IO::write_from_io;
    virtual void write(const void *source, size_t length) override;
    virtual void write_from_io(IO &source, size_t length) override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
