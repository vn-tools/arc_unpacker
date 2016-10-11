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

#include "algo/endian.h"
#include "io/base_byte_stream.h"

namespace au {
namespace dec {
namespace unity {

    template<typename T, size_t n> struct custom_stream_read_impl final
    {
        T operator()(
            io::BaseByteStream &original_stream,
            const algo::Endianness endianness)
        {
            return endianness == algo::Endianness::LittleEndian
                ? original_stream.read_le<T>()
                : original_stream.read_be<T>();
        }
    };

    template<typename T> struct custom_stream_read_impl<T, 1> final
    {
        T operator()(
            io::BaseByteStream &original_stream,
            const algo::Endianness endianness)
        {
            return original_stream.read<T>();
        }
    };

    class CustomStream final
    {
    public:
        CustomStream(io::BaseByteStream &original_stream) :
            endianness(algo::Endianness::BigEndian),
            original_stream(original_stream)
        {
        }

        void set_endianness(const algo::Endianness new_endianness)
        {
            endianness = new_endianness;
        }

        void align(const size_t n)
        {
            while (original_stream.pos() % n != 0)
                original_stream.skip(1);
        }

        bstr read(const size_t n)
        {
            return original_stream.read(n);
        }

        bstr read_to_zero(const size_t n)
        {
            return original_stream.read_to_zero(n);
        }

        template<typename T> T read()
        {
            return custom_stream_read_impl<T, sizeof(T)>()(
                original_stream, endianness);
        }

        bstr read_to_zero()
        {
            return original_stream.read_to_zero();
        }

        void skip(const size_t n)
        {
            original_stream.skip(n);
        }

    private:
        algo::Endianness endianness;
        io::BaseByteStream &original_stream;
    };

} } }
