#ifndef AU_IO_BUFFERED_IO_H
#define AU_IO_BUFFERED_IO_H
#include <memory>
#include "io/io.h"

namespace au {
namespace io {

    class BufferedIO final : public IO
    {
    public:
        BufferedIO();
        BufferedIO(const char *buffer, size_t buffer_size);
        BufferedIO(const bstr &buffer);
        BufferedIO(IO &other_io, size_t size);
        BufferedIO(IO &other_io);
        ~BufferedIO();

        virtual size_t size() const override;
        virtual size_t tell() const override;
        virtual void seek(size_t offset) override;
        virtual void skip(int offset) override;
        virtual void truncate(size_t new_size) override;

        using IO::read;
        virtual void read(void *destination, size_t size) override;

        using IO::write;
        using IO::write_from_io;
        virtual void write(const void *source, size_t size) override;
        virtual void write_from_io(IO &source, size_t size) override;

        void reserve(size_t count);
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }

#endif
