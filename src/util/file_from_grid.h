#pragma once

#include <memory>
#include "io/file.h"
#include "pix/grid.h"

namespace au {
namespace util {

    std::unique_ptr<io::File> file_from_grid(
        const pix::Grid &pixels, const std::string &name);

} }
