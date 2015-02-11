#include "compat/open.h"

#ifdef _WIN32
    #include <Windows.h>
    #include "compat/winutf8.h"

    FILE *compat_open(const char *path, const char *mode)
    {
        auto wpath = string2wstring(std::string(path));
        auto wmode = string2wstring(std::string(mode));
        return _wfopen(wpath.c_str(), wmode.c_str());
    }

#else

    FILE *compat_open(const char *path, const char *rb)
    {
        return fopen(path, rb);
    }

#endif
