#pragma once

#include <functional>
#include "types.h"

namespace au {
namespace io {

    class Stream
    {
    public:
        virtual ~Stream();

        virtual size_t size() const = 0;
        virtual size_t tell() const = 0;
        virtual Stream &seek(size_t offset) = 0;
        virtual Stream &skip(int offset) = 0;
        virtual void truncate(size_t new_size) = 0;
        void peek(size_t offset, std::function<void()> func);
        bool eof() const;

        bstr read(size_t bytes);
        bstr read_to_zero();
        bstr read_to_zero(size_t bytes);
        bstr read_to_eof();
        bstr read_line();
        virtual u8 read_u8();
        virtual u16 read_u16_le();
        virtual u16 read_u16_be();
        virtual u32 read_u32_le();
        virtual u32 read_u32_be();
        virtual u64 read_u64_le();
        virtual u64 read_u64_be();

        virtual void write(const bstr &bytes);
        virtual void write_u8(u8);
        virtual void write_u16_le(u16);
        virtual void write_u16_be(u16);
        virtual void write_u32_le(u32);
        virtual void write_u32_be(u32);
        virtual void write_u64_le(u64);
        virtual void write_u64_be(u64);

    protected:
        virtual void read_impl(void *input, size_t size) = 0;
        virtual void write_impl(const void *str, size_t size) = 0;
    };

} }
