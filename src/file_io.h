#ifndef FILE_IO
#define FILE_IO
#include <memory>
#include <string>
#include "io.h"

class FileIO final : public IO
{
public:
    FileIO(const std::string path, const std::string read_mode);
    ~FileIO();

    virtual size_t size() const override;
    virtual size_t tell() const override;
    virtual void seek(size_t offset) override;
    virtual void skip(size_t offset) override;
    virtual void truncate(size_t new_size) override;

    using IO::read;
    virtual void read(void *destination, size_t length) override;

    using IO::write;
    virtual void write(const void *source, size_t length) override;
    virtual void write_from_io(IO &source, size_t length) override;
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
