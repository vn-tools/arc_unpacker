#pragma once

#include <functional>

void suppress_output(std::function<void()> callback);
