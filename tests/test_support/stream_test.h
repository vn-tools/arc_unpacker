#pragma once

#include <memory>
#include "io/istream.h"

namespace au {
namespace tests {

    void stream_test(
        const std::function<std::unique_ptr<io::IStream>()> &factory,
        const std::function<void()> &cleanup);

} }
