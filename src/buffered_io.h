#ifndef BUFFERED_IO
#define BUFFERED_IO
#include <memory>
#include <string>
#include "io.h"

class BufferedIO final : public IO
{
public:
    BufferedIO();
    BufferedIO(const char *buffer, size_t buffer_size);
    BufferedIO(const std::string &buffer);
    ~BufferedIO();

    virtual size_t size() const override;
    virtual size_t tell() const override;
    virtual void seek(size_t offset) override;
    virtual void skip(ssize_t offset) override;
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
