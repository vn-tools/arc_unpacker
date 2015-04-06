#ifndef TEST_SUPPORT_SUPPRESS_OUTPUT_H
#define TEST_SUPPORT_SUPPRESS_OUTPUT_H
#include <functional>

void suppress_output(std::function<void()> callback);

#endif
