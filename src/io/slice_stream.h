#pragma once

#include <memory>
#include "err.h"
#include "io/base_byte_stream.h"

namespace au {
namespace io {

    class SliceStream final : public BaseByteStream
    {
    public:
        SliceStream(io::BaseByteStream &parent_stream, const uoff_t offset);
        SliceStream(
            io::BaseByteStream &parent_stream,
            const uoff_t offset,
            const uoff_t size);
        ~SliceStream();

        uoff_t size() const override;
        uoff_t pos() const override;
        std::unique_ptr<BaseByteStream> clone() const override;

    protected:
        void read_impl(void *destination, const size_t size) override;
        void write_impl(const void *source, const size_t size) override;
        void seek_impl(const uoff_t offset) override;
        void resize_impl(const uoff_t new_size) override;

    private:
        std::unique_ptr<io::BaseByteStream> parent_stream;
        const uoff_t slice_offset;
        const uoff_t slice_size;
    };

} }
