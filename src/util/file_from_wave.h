#pragma once

#include <memory>
#include "sfx/wave.h"
#include "file.h"

namespace au {
namespace util {

    std::unique_ptr<File> file_from_wave(
        const sfx::Wave &wave, const std::string &name);

} }
