#pragma once

#include <memory>
#include "io/base_byte_stream.h"

namespace au {
namespace tests {

    void stream_test(
        const std::function<std::unique_ptr<io::BaseByteStream>()> &factory,
        const std::function<void()> &cleanup);

} }
