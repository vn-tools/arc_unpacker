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
