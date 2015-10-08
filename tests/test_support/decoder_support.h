#pragma once

#include <boost/filesystem/path.hpp>
#include <vector>
#include "fmt/archive_decoder.h"
#include "fmt/file_decoder.h"
#include "fmt/image_decoder.h"

namespace au {
namespace tests {

    std::vector<std::shared_ptr<File>> unpack(
        const fmt::ArchiveDecoder &decoder, File &input_file);

    std::unique_ptr<File> decode(
        const fmt::FileDecoder &decoder, File &input_file);

    pix::Grid decode(const fmt::ImageDecoder &decoder, File &input_file);

} }
