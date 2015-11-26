#pragma once

#include <memory>
#include "io/file.h"
#include "sfx/audio.h"

namespace au {
namespace util {

    std::unique_ptr<io::File> file_from_audio(
        const sfx::Audio &audio, const io::path &name);

} }
