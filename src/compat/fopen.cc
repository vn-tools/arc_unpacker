#include <cstdio>
#include "compat/fopen.h"
#include "util/encoding.h"

FILE *fopen(const boost::filesystem::path &path, const char *mode)
{
    #ifdef _WIN32
        std::string cmode
            = convert_encoding(std::string(mode), "UTF-8", "UTF-16LE");
        std::wstring widemode(
            reinterpret_cast<const wchar_t*>(cmode.c_str()),
            cmode.length() / 2);
        return _wfopen(path.native().c_str(), widemode.c_str());
    #else
        return std::fopen(path.native().c_str(), mode);
    #endif
}
