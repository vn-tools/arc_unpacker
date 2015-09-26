#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include "file.h"
#include "pix/grid.h"

namespace au {
namespace tests {

    std::shared_ptr<pix::Grid> image_from_file(File &file);

    std::shared_ptr<pix::Grid> image_from_path(
        const boost::filesystem::path &path);

    void compare_images(
        const pix::Grid &expected_image, const pix::Grid &actual_image);

    void compare_images(
        const std::vector<std::shared_ptr<pix::Grid>> &expected_images,
        const std::vector<std::shared_ptr<pix::Grid>> &actual_images);

    void compare_images(
        const std::vector<std::shared_ptr<pix::Grid>> &expected_images,
        const std::vector<std::shared_ptr<File>> &actual_files);

} }
