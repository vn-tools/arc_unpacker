#pragma once

#include <memory>
#include "io/file.h"
#include "pix/grid.h"

namespace au {
namespace tests {

    std::shared_ptr<pix::Grid> image_from_file(io::File &file);
    std::shared_ptr<pix::Grid> image_from_path(const io::path &path);

    void compare_images(
        const pix::Grid &expected_image, const pix::Grid &actual_image);

    void compare_images(
        const std::vector<std::shared_ptr<pix::Grid>> &expected_images,
        const std::vector<std::shared_ptr<pix::Grid>> &actual_images);

    void compare_images(
        const std::vector<std::shared_ptr<pix::Grid>> &expected_images,
        const std::vector<std::shared_ptr<io::File>> &actual_files);

} }
