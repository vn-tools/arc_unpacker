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

#include "dec/aoi/aog_audio_decoder.h"

using namespace au;
using namespace au::dec::aoi;

static const bstr aoi_magic = "AoiOgg"_b;
static const bstr ogg_magic = "OggS"_b;

bool AogAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(aoi_magic.size()) == aoi_magic
        || (input_file.stream.seek(0).read(ogg_magic.size()) == ogg_magic
            && input_file.path.has_extension("aog"));
}

std::unique_ptr<io::File> AogAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    return std::make_unique<io::File>(
        io::path(input_file.path).change_extension("ogg"),
        input_file.stream.seek(0).read(aoi_magic.size()) == aoi_magic
            ? input_file.stream.seek(0x2C).read_to_eof()
            : input_file.stream.seek(0).read_to_eof());
}

static auto _ = dec::register_decoder<AogAudioDecoder>("aoi/aog");
