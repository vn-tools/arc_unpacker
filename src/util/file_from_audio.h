#pragma once

#include <memory>
#include "io/file.h"
#include "res/audio.h"

namespace au {
namespace util {

    std::unique_ptr<io::File> file_from_audio(
        const res::Audio &audio, const io::path &name);

} }
