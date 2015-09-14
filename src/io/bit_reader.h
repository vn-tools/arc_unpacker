#pragma once

#include <memory>
#include "io/io.h"

namespace au {
namespace io {

    class BitReader final
    {
    public:
        BitReader(IO &io);
        BitReader(const bstr &buffer);
        BitReader(const char *buffer, size_t buffer_size);
        ~BitReader();

        void seek(size_t pos);
        void skip(int offset);
        bool eof() const;
        size_t tell() const;
        size_t size() const;
        unsigned int get(size_t n);
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
