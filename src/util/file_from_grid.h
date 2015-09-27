#pragma once

#include <memory>
#include "file.h"
#include "pix/grid.h"

namespace au {
namespace util {

    std::unique_ptr<File> file_from_grid(
        const pix::Grid &pixels, const std::string &name);

} }
