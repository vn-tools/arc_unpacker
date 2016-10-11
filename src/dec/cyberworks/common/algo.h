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

#include "dec/cyberworks/dat_plugin.h"
#include "io/memory_byte_stream.h"
#include "logger.h"

namespace au {
namespace dec {
namespace cyberworks {
namespace common {

    void decode_data(
        const Logger &logger,
        const bstr &type,
        bstr &data,
        const DatPlugin &plugin);

    u32 read_obfuscated_number(io::BaseByteStream &input_stream);

} } } }
