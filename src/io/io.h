#pragma once

#include <functional>
#include "types.h"

namespace au {
namespace io {

    class IO
    {
    public:
        virtual ~IO();

        virtual size_t size() const = 0;
        virtual size_t tell() const = 0;
        virtual IO &seek(size_t offset) = 0;
        virtual IO &skip(int offset) = 0;
        virtual void truncate(size_t new_size) = 0;
        void peek(size_t offset, std::function<void()> func);
        bool eof() const;

        virtual void read(void *input, size_t size) = 0;
        virtual void write(const void *str, size_t size) = 0;

        bstr read(size_t bytes);
        bstr read_to_zero();
        bstr read_to_zero(size_t bytes);
        bstr read_to_eof();
        bstr read_line();
        u8 read_u8();
        u16 read_u16_le();
        u16 read_u16_be();
        u32 read_u32_le();
        u32 read_u32_be();
        u64 read_u64_le();
        u64 read_u64_be();

        void write(const bstr &bytes);
        void write_u8(u8);
        void write_u16_le(u16);
        void write_u16_be(u16);
        void write_u32_le(u32);
        void write_u32_be(u32);
        void write_u64_le(u64);
        void write_u64_be(u64);
    };

} }
