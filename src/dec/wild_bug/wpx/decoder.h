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

#include <memory>
#include "io/base_byte_stream.h"

namespace au {
namespace dec {
namespace wild_bug {
namespace wpx {

    class Decoder final
    {
    public:
        Decoder(io::BaseByteStream &input_stream);
        ~Decoder();

        std::string get_tag() const;
        const std::vector<u8> get_sections() const;
        bool has_section(u8 section_id) const;
        bstr read_plain_section(u8 section_id);
        bstr read_compressed_section(u8 section_id);
        bstr read_compressed_section(
            u8 section_id, s8 quant_size, const std::array<size_t, 8> &offsets);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
