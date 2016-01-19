#pragma once

#include <memory>
#include <string>
#include "io/base_stream.h"
#include "io/path.h"

namespace au {
namespace io {

    enum class FileMode : u8
    {
        Read = 1,
        Write = 2,
    };

    class FileStream final : public BaseStream
    {
    public:
        FileStream(const path &path, const FileMode mode);
        ~FileStream();

        size_t size() const override;
        size_t tell() const override;
        IStream &seek(const size_t offset) override;
        IStream &truncate(const size_t new_size) override;

        std::unique_ptr<IStream> clone() const override;

    protected:
        void read_impl(void *destination, const size_t size) override;
        void write_impl(const void *source, const size_t size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
