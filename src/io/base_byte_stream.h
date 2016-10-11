// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

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
        BaseByteStream &skip(const soff_t offset) override
        {
            BaseStream::skip(offset);
            return *this;
        }

        BaseByteStream &seek(const uoff_t offset) override
        {
            seek_impl(offset);
            return *this;
        }

        BaseByteStream &resize(const uoff_t new_size) override
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
        virtual void seek_impl(const uoff_t offset) = 0;
        virtual void resize_impl(const uoff_t new_size) = 0;
    };

} }
