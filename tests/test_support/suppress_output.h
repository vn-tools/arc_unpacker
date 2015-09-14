#pragma once

#include <functional>

namespace au {
namespace tests {

    void suppress_output(std::function<void()> callback);

} }
