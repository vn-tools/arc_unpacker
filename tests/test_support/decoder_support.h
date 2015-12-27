#pragma once

#include "fmt/base_archive_decoder.h"
#include "fmt/base_audio_decoder.h"
#include "fmt/base_file_decoder.h"
#include "fmt/base_image_decoder.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<io::File>> unpack(
        const fmt::BaseArchiveDecoder &decoder, io::File &input_file);

    std::unique_ptr<io::File> decode(
        const fmt::BaseFileDecoder &decoder, io::File &input_file);

    res::Image decode(
        const fmt::BaseImageDecoder &decoder, io::File &input_file);

    res::Audio decode(
        const fmt::BaseAudioDecoder &decoder, io::File &input_file);

} }
