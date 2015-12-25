#pragma once

#include <functional>
#include <memory>
#include "types.h"

namespace au {
namespace io {

    class Stream
    {
    public:
        virtual ~Stream();

        virtual size_t size() const = 0;
        virtual size_t tell() const = 0;
        virtual Stream &seek(const size_t offset) = 0;
        virtual Stream &skip(const int offset) = 0;
        virtual Stream &truncate(const size_t new_size) = 0;
        Stream &peek(const size_t offset, const std::function<void()> func);
        bool eof() const;

        bstr read(const size_t bytes);
        bstr read_to_zero();
        bstr read_to_zero(const size_t bytes);
        bstr read_to_eof();
        bstr read_line();
        virtual u8 read_u8();
        virtual u16 read_u16_le();
        virtual u16 read_u16_be();
        virtual u32 read_u32_le();
        virtual u32 read_u32_be();
        virtual u64 read_u64_le();
        virtual u64 read_u64_be();
        virtual f32 read_f32_le();
        virtual f32 read_f32_be();
        virtual f64 read_f64_le();
        virtual f64 read_f64_be();

        virtual Stream &write(const bstr &bytes);
        virtual Stream &write_u8(u8);
        virtual Stream &write_u16_le(u16);
        virtual Stream &write_u16_be(u16);
        virtual Stream &write_u32_le(u32);
        virtual Stream &write_u32_be(u32);
        virtual Stream &write_u64_le(u64);
        virtual Stream &write_u64_be(u64);

        virtual std::unique_ptr<Stream> clone() const = 0;

    protected:
        virtual void read_impl(void *input, const size_t size) = 0;
        virtual void write_impl(const void *str, const size_t size) = 0;
    };

} }
