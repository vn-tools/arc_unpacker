#ifndef COMPAT_MAIN_H
#define COMPAT_MAIN_H
#include <string>
#include <vector>

std::vector<std::string> get_arguments(int argc, const char **arg);
std::vector<std::string> get_arguments(int argc, const wchar_t **argv);
void init_fs_utf8();

#ifdef _WIN32
    #define ENTRY_POINT(x) extern "C" int wmain(int argc, const wchar_t **argv) \
    { \
        std::vector<std::string> arguments = get_arguments(argc, argv); \
        init_fs_utf8(); \
        x \
    }
#else
    #define ENTRY_POINT(x) int main(int argc, const char **argv) \
    { \
        std::vector<std::string> arguments = get_arguments(argc, argv); \
        init_fs_utf8(); \
        x \
    }
#endif
#endif
