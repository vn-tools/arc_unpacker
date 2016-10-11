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

#include "io/base_byte_stream.h"
#include "io/base_stream.h"
#include "types.h"

namespace au {
namespace io {

    class BaseBitStream : public BaseStream
    {
    public:
        BaseBitStream(const bstr &input);
        BaseBitStream(io::BaseByteStream &input_stream);
        virtual ~BaseBitStream() = 0;

        uoff_t pos() const override;
        uoff_t size() const override;
        BaseStream &seek(const uoff_t offset) override;
        BaseStream &resize(const uoff_t new_size) override;

        u32 read_gamma(const bool stop_mark);
        virtual u32 read(const size_t n) = 0;
        virtual void flush();
        virtual void write(const size_t bits, const u32 value);

    protected:
        u64 buffer;
        size_t bits_available;
        size_t position;
        std::unique_ptr<io::BaseByteStream> own_stream_holder;
        io::BaseByteStream *input_stream;
    };

} }
