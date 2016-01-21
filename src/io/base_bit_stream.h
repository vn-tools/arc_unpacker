#pragma once

#include "io/base_byte_stream.h"
#include "io/base_stream.h"
#include "types.h"

namespace au {
namespace io {

    class BaseBitStream : public BaseStream
    {
    public:
        BaseBitStream(const bstr &input);
        BaseBitStream(io::BaseByteStream &input_stream);
        virtual ~BaseBitStream() = 0;

        size_t tell() const override;
        size_t size() const override;
        BaseStream &seek(const size_t offset) override;
        BaseStream &truncate(const size_t new_size) override;

        virtual u32 read(const size_t n) = 0;
        u32 read_gamma(const bool stop_mark);

    protected:
        u64 buffer;
        size_t bits_available;
        size_t position;
        std::unique_ptr<io::BaseByteStream> own_stream_holder;
        io::BaseByteStream *input_stream;
    };

} }
