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

#include "dec/crowd/zbm_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::dec::crowd;

static const auto magic = "SZDD"_b;

bool ZbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic
        && input_file.path.has_extension("zbm");
}

res::Image ZbmImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(10);
    const auto size_orig = input_file.stream.read_le<u32>();

    algo::pack::BytewiseLzssSettings settings;
    settings.initial_dictionary_pos = 0xFF0;
    auto data = algo::pack::lzss_decompress(
        input_file.stream.read_to_eof(), size_orig, settings);
    for (const auto i : algo::range(std::min<size_t>(100, data.size())))
        data[i] ^= 0xFF;

    const auto bmp_decoder = dec::microsoft::BmpImageDecoder();
    io::File bmp_file("dummy.bmp", data);
    return bmp_decoder.decode(logger, bmp_file);
}

static auto _ = dec::register_decoder<ZbmImageDecoder>("crowd/zbm");
