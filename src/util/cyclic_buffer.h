#pragma once

#include <memory>
#include "types.h"

namespace au {
namespace util {

    class CyclicBuffer final
    {
    public:
        CyclicBuffer(const size_t size, const size_t start_pos = 0);
        ~CyclicBuffer();
        size_t size() const;
        size_t start() const;
        size_t pos() const;
        void operator <<(const bstr &s);
        void operator <<(const u8 c);
        u8 &operator [](const size_t n);
        const u8 &operator [](const size_t n) const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
