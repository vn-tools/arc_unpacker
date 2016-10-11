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

#include "dec/crowd/cwp_image_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::crowd;

static const auto magic = "CWDP"_b;

bool CwpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image CwpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto ihdr = input_file.stream.read(13);
    const auto ihdr_crc = input_file.stream.read_be<u32>();
    const auto first_idat_size = input_file.stream.read_be<u32>();
    const auto rest = input_file.stream.read(
        input_file.stream.size() - 1 - input_file.stream.pos());

    io::File png_file("dummy.png", ""_b);
    png_file.stream.write("\x89""PNG"_b);
    png_file.stream.write("\x0D\x0A\x1A\x0A"_b);
    png_file.stream.write_be<u32>(ihdr.size());
    png_file.stream.write("IHDR"_b);
    png_file.stream.write(ihdr);
    png_file.stream.write_be<u32>(ihdr_crc);

    png_file.stream.write_be<u32>(first_idat_size);
    png_file.stream.write("IDAT"_b);
    png_file.stream.write(rest);

    png_file.stream.write_be<u32>(0);
    png_file.stream.write("IEND"_b);
    png_file.stream.write("\xAE\x42\x60\x82"_b);

    const auto png_decoder = dec::png::PngImageDecoder();
    auto image = png_decoder.decode(logger, png_file);
    for (auto &c : image)
    {
        std::swap(c.b, c.r);
        c.a = 0xFF;
    }
    return image;
}

static auto _ = dec::register_decoder<CwpImageDecoder>("crowd/cwp");
