#pragma once

#include "types.h"

namespace au {
namespace io {

    class IBitReader
    {
    public:
        virtual ~IBitReader() {}
        virtual void seek(const size_t pos) = 0;
        virtual void skip(const int offset) = 0;
        virtual size_t tell() const = 0;
        virtual size_t size() const = 0;
        virtual bool eof() const = 0;
        virtual u32 get(const size_t n) = 0;
        virtual u32 get_gamma(const bool stop_mark) = 0;
    };

} }
