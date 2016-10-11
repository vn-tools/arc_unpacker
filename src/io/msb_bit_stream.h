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

#include "io/base_bit_stream.h"
#include "io/base_byte_stream.h"

namespace au {
namespace io {

    class MsbBitStream final : public BaseBitStream
    {
    public:
        MsbBitStream(const bstr &input);
        MsbBitStream(io::BaseByteStream &input_stream);
        ~MsbBitStream();
        u32 read(const size_t bits) override;
        void flush() override;
        void write(const size_t bits, const u32 value) override;
    private:
        bool dirty;
    };

} }
