#pragma once

#include <memory>
#include <string>
#include "io/path.h"
#include "io/stream.h"

namespace au {
namespace io {

    enum class FileMode : u8
    {
        Read = 1,
        Write = 2,
    };

    class FileStream final : public Stream
    {
    public:
        FileStream(const path &path, const FileMode mode);
        ~FileStream();

        size_t size() const override;
        size_t tell() const override;
        Stream &seek(size_t offset) override;
        Stream &skip(int offset) override;
        Stream &truncate(size_t new_size) override;

    protected:
        void read_impl(void *destination, size_t size) override;
        void write_impl(const void *source, size_t size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
