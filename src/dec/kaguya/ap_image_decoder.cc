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

#include "dec/kaguya/ap_image_decoder.h"

using namespace au;
using namespace au::dec::kaguya;

static const bstr magic = "AP"_b;

bool ApImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    return width * height * 4 + 2 + 4 + 4 + 2 == input_file.stream.size();
}

res::Image ApImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(2);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    input_file.stream.skip(2);
    const auto data = input_file.stream.read_to_eof();
    return res::Image(width, height, data, res::PixelFormat::BGRA8888)
        .flip_vertically();
}

static auto _ = dec::register_decoder<ApImageDecoder>("kaguya/ap");
