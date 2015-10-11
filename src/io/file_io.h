#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "io/io.h"

namespace au {
namespace io {

    enum class FileMode : u8
    {
        Read = 1,
        Write = 2,
    };

    class FileIO final : public IO
    {
    public:
        FileIO(const boost::filesystem::path &path, const FileMode mode);
        ~FileIO();

        size_t size() const override;
        size_t tell() const override;
        void seek(size_t offset) override;
        void skip(int offset) override;
        void truncate(size_t new_size) override;

        using IO::read;
        void read(void *destination, size_t size) override;

        using IO::write;
        using IO::write_from_io;
        void write(const void *source, size_t size) override;
        void write_from_io(IO &source, size_t size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
