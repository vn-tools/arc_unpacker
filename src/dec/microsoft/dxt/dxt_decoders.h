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
#include "res/image.h"

namespace au {
namespace dec {
namespace microsoft {
namespace dxt {

    std::unique_ptr<res::Image> decode_dxt1(
        io::BaseByteStream &input_stream,
        const size_t width,
        const size_t height);

    std::unique_ptr<res::Image> decode_dxt3(
        io::BaseByteStream &input_stream,
        const size_t width,
        const size_t height);

    std::unique_ptr<res::Image> decode_dxt5(
        io::BaseByteStream &input_stream,
        const size_t width,
        const size_t height);

} } } }
