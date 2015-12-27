#pragma once

#include <memory>
#include "io/stream.h"

namespace au {
namespace io {

    class IBitReader
    {
    public:
        virtual ~IBitReader() { }

        virtual void seek(const size_t pos) = 0;
        virtual void skip(const int offset) = 0;
        virtual size_t tell() const = 0;
        virtual size_t size() const = 0;
        virtual bool eof() const = 0;
        virtual u32 get(const size_t n) = 0;
    };

    class BaseBitReader : public IBitReader
    {
    public:
        BaseBitReader(const bstr &input);
        BaseBitReader(io::Stream &input_stream);
        virtual ~BaseBitReader() { }

        void seek(const size_t pos) override;
        void skip(const int offset) override;
        size_t tell() const override;
        size_t size() const override;
        bool eof() const override;

    protected:
        u64 buffer;
        size_t bits_available;
        size_t position;
        std::unique_ptr<io::Stream> own_stream_holder;
        io::Stream *input_stream;
    };

    class LsbBitReader final : public BaseBitReader
    {
    public:
        LsbBitReader(const bstr &input);
        LsbBitReader(io::Stream &input_stream);
        u32 get(const size_t n) override;
    };

    class MsbBitReader final : public BaseBitReader
    {
    public:
        MsbBitReader(const bstr &input);
        MsbBitReader(io::Stream &input_stream);
        u32 get(const size_t n) override;
    };

} }
