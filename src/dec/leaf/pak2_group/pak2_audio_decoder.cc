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

#include "dec/leaf/pak2_group/pak2_audio_decoder.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "\x03\x95\xAD\x4B"_b;

bool Pak2AudioDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(4);
    return input_file.stream.read(4) == magic;
}

std::unique_ptr<io::File> Pak2AudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(12);
    const auto size_comp = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    const auto data = input_file.stream.read(size_comp);
    if (input_file.stream.left())
        logger.warn("Extra data after EOF.\n");
    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<Pak2AudioDecoder>("leaf/pak2-audio");
