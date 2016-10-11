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

#include "dec/scene_player/pmw_audio_decoder.h"
#include "algo/binary.h"
#include "algo/pack/zlib.h"

using namespace au;
using namespace au::dec::scene_player;

bool PmwAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("pmw");
}

std::unique_ptr<io::File> PmwAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto data
        = algo::pack::zlib_inflate(
            algo::unxor(
                input_file.stream.seek(0).read_to_eof(), 0x21));
    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("wav");
    return output_file;
}

static auto _ = dec::register_decoder<PmwAudioDecoder>("scene-player/pmw");
