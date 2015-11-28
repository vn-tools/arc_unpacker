#pragma once

#include <memory>
#include "io/stream.h"

namespace au {
namespace tests {

    void stream_test(
        const std::function<std::unique_ptr<io::Stream>()> &factory,
        const std::function<void()> &cleanup);

} }
