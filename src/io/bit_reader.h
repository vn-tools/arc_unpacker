#pragma once

#include <memory>
#include "io/stream.h"

namespace au {
namespace io {

    class BitReader final
    {
    public:
        BitReader(Stream &stream);
        BitReader(const bstr &buffer);
        BitReader(const char *buffer, const size_t buffer_size);
        ~BitReader();

        void seek(const size_t pos);
        void skip(const int offset);
        bool eof() const;
        size_t tell() const;
        size_t size() const;
        unsigned int get(const size_t n);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
