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

#include "dec/gsd/bmz_image_decoder.h"
#include "algo/pack/zlib.h"
#include "dec/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::dec::gsd;

static const bstr magic = "ZLC3"_b;

bool BmzImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image BmzImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto data = algo::pack::zlib_inflate(input_file.stream.read_to_eof());
    const auto pseudo_file = std::make_unique<io::File>(input_file.path, data);
    return dec::microsoft::BmpImageDecoder().decode(logger, *pseudo_file);
}

static auto _ = dec::register_decoder<BmzImageDecoder>("gsd/bmz");
