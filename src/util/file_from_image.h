#pragma once

#include <memory>
#include "io/file.h"
#include "pix/image.h"

namespace au {
namespace util {

    std::unique_ptr<io::File> file_from_image(
        const pix::Image &pixels, const io::path &name);

} }
