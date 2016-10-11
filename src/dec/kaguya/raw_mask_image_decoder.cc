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

#include "dec/kaguya/raw_mask_image_decoder.h"

using namespace au;
using namespace au::dec::kaguya;

bool RawMaskImageDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    return input_file.stream.size() == width * height + 8;
}

res::Image RawMaskImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    return res::Image(
        width,
        height,
        input_file.stream.read_to_eof(),
        res::PixelFormat::Gray8);
}

static auto _
    = dec::register_decoder<RawMaskImageDecoder>("kaguya/raw-mask");
