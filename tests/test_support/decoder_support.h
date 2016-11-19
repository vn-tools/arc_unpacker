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

#pragma once

#include "dec/base_archive_decoder.h"
#include "dec/base_audio_decoder.h"
#include "dec/base_file_decoder.h"
#include "dec/base_image_decoder.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<io::File>> unpack(
        const au::dec::BaseArchiveDecoder &decoder, io::File &input_file);

    std::unique_ptr<io::File> decode(
        const au::dec::BaseFileDecoder &decoder, io::File &input_file);

    res::Image decode(
        const au::dec::BaseImageDecoder &decoder, io::File &input_file);

    res::Audio decode(
        const au::dec::BaseAudioDecoder &decoder, io::File &input_file);

} }
