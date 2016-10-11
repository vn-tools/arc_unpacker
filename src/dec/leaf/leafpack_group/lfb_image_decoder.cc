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

#include "dec/leaf/leafpack_group/lfb_image_decoder.h"
#include "dec/leaf/common/custom_lzss.h"
#include "dec/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::dec::leaf;

bool LfbImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("lfb");
}

res::Image LfbImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto data = common::custom_lzss_decompress(
        input_file.stream.read_to_eof(), size_orig);
    const auto pseudo_file = std::make_unique<io::File>(input_file.path, data);
    return dec::microsoft::BmpImageDecoder().decode(logger, *pseudo_file);
}

static auto _ = dec::register_decoder<LfbImageDecoder>("leaf/lfb");
