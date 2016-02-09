#pragma once

#include <memory>
#include "err.h"
#include "io/base_byte_stream.h"
#include "io/base_stream.h"

namespace au {
namespace io {

    class MemoryStream final : public BaseByteStream
    {
    public:
        MemoryStream();
        MemoryStream(const char *buffer, const size_t buffer_size);
        MemoryStream(const bstr &buffer);
        MemoryStream(BaseByteStream &other_stream, const size_t size);
        MemoryStream(BaseByteStream &other_stream);
        ~MemoryStream();

        size_t size() const override;
        size_t tell() const override;

        BaseByteStream &reserve(const size_t count);

        std::unique_ptr<BaseByteStream> clone() const override;

    protected:
        void read_impl(void *destination, const size_t size) override;
        void write_impl(const void *source, const size_t size) override;
        void seek_impl(const size_t offset) override;
        void resize_impl(const size_t new_size) override;

    private:
        MemoryStream(const std::shared_ptr<bstr> buffer);

        std::shared_ptr<bstr> buffer;
        size_t buffer_pos;
    };

} }
