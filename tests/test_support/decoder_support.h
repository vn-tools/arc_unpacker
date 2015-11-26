#pragma once

#include "fmt/archive_decoder.h"
#include "fmt/audio_decoder.h"
#include "fmt/file_decoder.h"
#include "fmt/image_decoder.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<io::File>> unpack(
        const fmt::ArchiveDecoder &decoder, io::File &input_file);

    std::unique_ptr<io::File> decode(
        const fmt::FileDecoder &decoder, io::File &input_file);

    pix::Grid decode(const fmt::ImageDecoder &decoder, io::File &input_file);

    std::unique_ptr<io::File> decode(
        const fmt::AudioDecoder &decoder, io::File &input_file);

} }
