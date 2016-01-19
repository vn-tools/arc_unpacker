#pragma once

#include <functional>
#include <memory>
#include "algo/endian.h"
#include "types.h"

namespace au {
namespace io {

    class IStream
    {
    public:
        virtual ~IStream() {}

        virtual size_t left() const = 0;
        virtual size_t size() const = 0;
        virtual size_t tell() const = 0;
        virtual IStream &seek(const size_t offset) = 0;
        virtual IStream &skip(const int offset) = 0;
        virtual IStream &truncate(const size_t new_size) = 0;
        virtual IStream &peek(
            const size_t offset, const std::function<void()> func) = 0;
        virtual bool eof() const = 0;

        virtual bstr read_to_zero() = 0;
        virtual bstr read_to_zero(const size_t bytes) = 0;
        virtual bstr read_to_eof() = 0;
        virtual bstr read_line() = 0;

        bstr read(const size_t bytes)
        {
            if (!bytes)
                return ""_b;
            bstr ret(bytes);
            read_impl(&ret[0], bytes);
            return ret;
        }

        template<typename T> T read()
        {
            static_assert(
                sizeof(T) == 1,
                "For multiple bytes, must specify endianness");
            T x;
            read_impl(&x, sizeof(x));
            return x;
        }

        template<typename T> T read_le()
        {
            static_assert(
                sizeof(T) > 1,
                "Endianness does not make sense for single bytes");
            T x;
            read_impl(&x, sizeof(x));
            return algo::from_little_endian(x);
        }

        template<typename T> T read_be()
        {
            static_assert(
                sizeof(T) > 1,
                "Endianness does not make sense for single bytes");
            T x;
            read_impl(&x, sizeof(x));
            return algo::from_big_endian(x);
        }

        io::IStream &write(const bstr &bytes)
        {
            if (!bytes.size())
                return *this;
            write_impl(bytes.get<char>(), bytes.size());
            return *this;
        }

        io::IStream &write(const std::string &bytes)
        {
            return write(bstr(bytes));
        }

        io::IStream &write(const char *bytes)
        {
            return write(bstr(bytes));
        }

        virtual io::IStream &write_zero_padded(
            const bstr &bytes, const size_t target_size) = 0;

        template<typename T> IStream &write(const T x)
        {
            static_assert(
                sizeof(T) == 1,
                "For multiple bytes, must specify endianness");
            write_impl(&x, sizeof(T));
            return *this;
        }

        template<typename T> IStream &write_le(const T x)
        {
            static_assert(
                sizeof(T) > 1,
                "Endianness does not make sense for single bytes");
            const auto y = algo::to_little_endian(x);
            write_impl(&y, sizeof(T));
            return *this;
        }

        template<typename T> io::IStream & write_be(const T x)
        {
            static_assert(
                sizeof(T) > 1,
                "Endianness does not make sense for single bytes");
            const auto y = algo::to_big_endian(x);
            write_impl(&y, sizeof(T));
            return *this;
        }

        virtual std::unique_ptr<IStream> clone() const = 0;

    protected:
        virtual void read_impl(void *input, const size_t size) = 0;
        virtual void write_impl(const void *str, const size_t size) = 0;
    };

} }
