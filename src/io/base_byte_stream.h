#pragma once

#include <functional>
#include <memory>
#include "algo/endian.h"
#include "io/base_stream.h"
#include "types.h"

namespace au {
namespace io {

    class BaseByteStream : public BaseStream
    {
    public:
        virtual ~BaseByteStream() = 0;

        // allow method chaining via BaseByteStream
        BaseByteStream &skip(const int offset) override
        {
            BaseStream::skip(offset);
            return *this;
        }

        BaseByteStream &seek(const size_t offset) override
        {
            seek_impl(offset);
            return *this;
        }

        BaseByteStream &resize(const size_t new_size) override
        {
            resize_impl(new_size);
            return *this;
        }

        bstr read_to_zero();
        bstr read_to_zero(const size_t bytes);
        bstr read_to_eof();
        bstr read_line();

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

        io::BaseByteStream &write(const bstr &bytes)
        {
            if (!bytes.size())
                return *this;
            write_impl(bytes.get<char>(), bytes.size());
            return *this;
        }

        io::BaseByteStream &write(const std::string &bytes)
        {
            return write(bstr(bytes));
        }

        io::BaseByteStream &write(const char *bytes)
        {
            return write(bstr(bytes));
        }

        io::BaseByteStream &write(io::BaseByteStream &other_stream);

        io::BaseByteStream &write(
            io::BaseByteStream &other_stream, const size_t size);

        io::BaseByteStream &write_zero_padded(
            const bstr &bytes, const size_t target_size);

        template<typename T> BaseByteStream &write(const T x)
        {
            static_assert(
                sizeof(T) == 1,
                "For multiple bytes, must specify endianness");
            write_impl(&x, sizeof(T));
            return *this;
        }

        template<typename T> BaseByteStream &write_le(const T x)
        {
            static_assert(
                sizeof(T) > 1,
                "Endianness does not make sense for single bytes");
            const auto y = algo::to_little_endian(x);
            write_impl(&y, sizeof(T));
            return *this;
        }

        template<typename T> BaseByteStream &write_be(const T x)
        {
            static_assert(
                sizeof(T) > 1,
                "Endianness does not make sense for single bytes");
            const auto y = algo::to_big_endian(x);
            write_impl(&y, sizeof(T));
            return *this;
        }

        virtual std::unique_ptr<BaseByteStream> clone() const = 0;

    protected:
        virtual void read_impl(void *input, const size_t size) = 0;
        virtual void write_impl(const void *str, const size_t size) = 0;
        virtual void seek_impl(const size_t offset) = 0;
        virtual void resize_impl(const size_t new_size) = 0;
    };

} }
