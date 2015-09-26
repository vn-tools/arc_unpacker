#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include "util/image.h"

namespace au {
namespace tests {

    std::shared_ptr<util::Image> image_from_file(File &file);

    std::shared_ptr<util::Image> image_from_path(
        const boost::filesystem::path &path);

    void compare_images(
        const util::Image &expected_image, const util::Image &actual_image);

    void compare_images(
        const std::vector<std::shared_ptr<util::Image>> &expected_images,
        const std::vector<std::shared_ptr<util::Image>> &actual_images);

    void compare_images(
        const std::vector<std::shared_ptr<util::Image>> &expected_images,
        const std::vector<std::shared_ptr<File>> &actual_files);

} }
