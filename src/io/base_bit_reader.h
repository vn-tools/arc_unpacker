#pragma once

#include <memory>
#include "io/ibit_reader.h"
#include "io/istream.h"

namespace au {
namespace io {

    class BaseBitReader : public IBitReader
    {
    public:
        BaseBitReader(const bstr &input);
        BaseBitReader(io::IStream &input_stream);
        virtual ~BaseBitReader() {}

        void seek(const size_t pos) override;
        void skip(const int offset) override;
        size_t tell() const override;
        size_t size() const override;
        bool eof() const override;
        u32 get_gamma(const bool stop_mark) override;

    protected:
        u64 buffer;
        size_t bits_available;
        size_t position;
        std::unique_ptr<io::IStream> own_stream_holder;
        io::IStream *input_stream;
    };

} }
