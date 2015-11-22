#pragma once

#include <memory>
#include "io/file.h"
#include "sfx/wave.h"

namespace au {
namespace util {

    std::unique_ptr<io::File> file_from_wave(
        const sfx::Wave &wave, const io::path &name);

} }
