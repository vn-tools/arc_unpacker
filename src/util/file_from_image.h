#pragma once

#include <memory>
#include "io/file.h"
#include "res/image.h"

namespace au {
namespace util {

    std::unique_ptr<io::File> file_from_image(
        const res::Image &image, const io::path &name);

} }
