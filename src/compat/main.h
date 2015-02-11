#ifndef COMPAT_MAIN_H
#define COMPAT_MAIN_H
#include <functional>
#include <string>
#include <vector>

int run_with_args(
    int argc,
    const char **argv,
    std::function<int(std::vector<std::string>)> main);

#endif
