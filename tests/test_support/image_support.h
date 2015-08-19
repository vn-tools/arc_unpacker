#ifndef AU_TESTS_TEST_SUPPORT_IMAGE_SUPPORT_H
#define AU_TESTS_TEST_SUPPORT_IMAGE_SUPPORT_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include "util/image.h"

namespace au {
namespace tests {

    void compare_images(
        const util::Image &expected_image,
        const util::Image &actual_image,
        int max_component_diff = 0);

    std::shared_ptr<util::Image> image_from_path(
        const boost::filesystem::path &path);

} }

#endif
