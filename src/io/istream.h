#pragma once

#include <functional>
#include <memory>
#include "types.h"

namespace au {
namespace io {

    class IStream
    {
    public:
        virtual ~IStream() {}

        virtual size_t size() const = 0;
        virtual size_t tell() const = 0;
        virtual IStream &seek(const size_t offset) = 0;
        virtual IStream &skip(const int offset) = 0;
        virtual IStream &truncate(const size_t new_size) = 0;
        virtual IStream &peek(
            const size_t offset, const std::function<void()> func) = 0;
        virtual bool eof() const = 0;

        virtual bstr read(const size_t bytes) = 0;
        virtual bstr read_to_zero() = 0;
        virtual bstr read_to_zero(const size_t bytes) = 0;
        virtual bstr read_to_eof() = 0;
        virtual bstr read_line() = 0;
        virtual u8 read_u8() = 0;
        virtual u16 read_u16_le() = 0;
        virtual u16 read_u16_be() = 0;
        virtual u32 read_u32_le() = 0;
        virtual u32 read_u32_be() = 0;
        virtual u64 read_u64_le() = 0;
        virtual u64 read_u64_be() = 0;
        virtual f32 read_f32_le() = 0;
        virtual f32 read_f32_be() = 0;
        virtual f64 read_f64_le() = 0;
        virtual f64 read_f64_be() = 0;

        virtual IStream &write(const bstr &bytes) = 0;
        virtual IStream &write_u8(u8) = 0;
        virtual IStream &write_u16_le(u16) = 0;
        virtual IStream &write_u16_be(u16) = 0;
        virtual IStream &write_u32_le(u32) = 0;
        virtual IStream &write_u32_be(u32) = 0;
        virtual IStream &write_u64_le(u64) = 0;
        virtual IStream &write_u64_be(u64) = 0;

        virtual std::unique_ptr<IStream> clone() const = 0;
    };

} }
