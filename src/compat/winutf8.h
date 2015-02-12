#ifndef COMPAT_WINUTF8_H
#define COMPAT_WINUTF8_H
#ifndef _WIN32
    #error Trying to include Windows-related headers for non-Windows system
#endif
#include <string>

std::wstring string2wstring(std::string str);
std::string wstring2string(std::wstring wstr);

#endif
