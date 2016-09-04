#pragma once

#include <memory>
#include "err.h"
#include "io/base_byte_stream.h"
#include "io/base_stream.h"

namespace au {
namespace io {

    class MemoryByteStream final : public BaseByteStream
    {
    public:
        MemoryByteStream();
        MemoryByteStream(const char *buffer, const size_t buffer_size);
        MemoryByteStream(const bstr &buffer);
        MemoryByteStream(BaseByteStream &other_stream, const size_t size);
        MemoryByteStream(BaseByteStream &other_stream);
        ~MemoryByteStream();

        uoff_t size() const override;
        uoff_t pos() const override;

        BaseByteStream &reserve(const uoff_t count);

        std::unique_ptr<BaseByteStream> clone() const override;

    protected:
        void read_impl(void *destination, const size_t size) override;
        void write_impl(const void *source, const size_t size) override;
        void seek_impl(const uoff_t offset) override;
        void resize_impl(const uoff_t new_size) override;

    private:
        MemoryByteStream(const std::shared_ptr<bstr> buffer);

        std::shared_ptr<bstr> buffer;
        uoff_t buffer_pos;
    };

} }
