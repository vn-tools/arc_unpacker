#pragma once

#include <memory>
#include "io/file.h"
#include "pix/image.h"

namespace au {
namespace tests {

    void compare_images(
        const pix::Image &expected_image,
        const pix::Image &actual_image);

    void compare_images(
        io::File &expected_file,
        const pix::Image &actual_image);

    void compare_images(
        io::File &expected_file,
        io::File &actual_file,
        const bool compare_file_names);

    void compare_images(
        const std::vector<std::shared_ptr<io::File>> &expected_images,
        const std::vector<std::shared_ptr<io::File>> &actual_images,
        const bool compare_file_names);

} }
