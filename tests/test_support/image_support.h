#pragma once

#include <memory>
#include "io/file.h"
#include "res/image.h"

namespace au {
namespace tests {

    res::Image get_opaque_test_image();

    void compare_images(
        const res::Image &actual_image,
        const res::Image &expected_image);

    void compare_images(
        const res::Image &actual_image,
        io::File &expected_file);

    void compare_images(
        io::File &actual_file,
        io::File &expected_file,
        const bool compare_file_paths);

    void compare_images(
        const std::vector<std::shared_ptr<io::File>> &actual_images,
        const std::vector<std::shared_ptr<io::File>> &expected_images,
        const bool compare_file_paths);

} }
