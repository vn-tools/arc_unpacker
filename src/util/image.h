#pragma once

#include <memory>
#include "file.h"
#include "pix/grid.h"

namespace au {
namespace util {

    std::unique_ptr<File> grid_to_boxed(
        const pix::Grid &pixels, const std::string &name);

} }
