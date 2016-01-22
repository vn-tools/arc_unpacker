#pragma once

#include <memory>
#include "io/file.h"
#include "res/audio.h"

namespace au {
namespace tests {

    void compare_audio(
        const res::Audio &actual_audio,
        const res::Audio &expected_audio);

    void compare_audio(
        const res::Audio &actual_audio,
        io::File &expected_file);

    void compare_audio(
        io::File &actual_file,
        io::File &expected_file,
        const bool compare_file_paths);

    void compare_audio(
        const std::vector<std::shared_ptr<io::File>> &actual_files,
        const std::vector<std::shared_ptr<io::File>> &expected_files,
        const bool compare_file_paths);


} }
