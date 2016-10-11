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

#include "dec/nekopack/masked_bmp_image_decoder.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "err.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::nekopack;

bool MaskedBmpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("alp");
}

res::Image MaskedBmpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto base_file = VirtualFileSystem::get_by_name(
        io::path(input_file.path).change_extension("bmp").name());
    if (!base_file)
        throw err::CorruptDataError("Missing base file");
    const auto bmp_decoder = dec::microsoft::BmpImageDecoder();
    auto base_image = bmp_decoder.decode(logger, *base_file);
    res::Image mask(
        base_image.width(),
        base_image.height(),
        input_file.stream.seek(0).read_to_eof(),
        res::PixelFormat::Gray8);
    base_image.apply_mask(mask);
    return base_image;
}

static auto _ = dec::register_decoder<MaskedBmpImageDecoder>(
    "nekopack/masked-bmp");
