#pragma once

#include "io/istream.h"

namespace au {
namespace io {

    class BaseStream : public IStream
    {
    public:
        virtual ~BaseStream() {}

        IStream &peek(
            const size_t offset, const std::function<void()> func) override;
        bool eof() const override;

        bstr read(const size_t bytes) override;
        bstr read_to_zero() override;
        bstr read_to_zero(const size_t bytes) override;
        bstr read_to_eof() override;
        bstr read_line() override;
        virtual u8 read_u8() override;
        virtual u16 read_u16_le() override;
        virtual u16 read_u16_be() override;
        virtual u32 read_u32_le() override;
        virtual u32 read_u32_be() override;
        virtual u64 read_u64_le() override;
        virtual u64 read_u64_be() override;
        virtual f32 read_f32_le() override;
        virtual f32 read_f32_be() override;
        virtual f64 read_f64_le() override;
        virtual f64 read_f64_be() override;

        virtual IStream &write(const bstr &bytes) override;
        virtual IStream &write_u8(u8) override;
        virtual IStream &write_u16_le(u16) override;
        virtual IStream &write_u16_be(u16) override;
        virtual IStream &write_u32_le(u32) override;
        virtual IStream &write_u32_be(u32) override;
        virtual IStream &write_u64_le(u64) override;
        virtual IStream &write_u64_be(u64) override;

    protected:
        virtual void read_impl(void *input, const size_t size) = 0;
        virtual void write_impl(const void *str, const size_t size) = 0;
    };

} }
