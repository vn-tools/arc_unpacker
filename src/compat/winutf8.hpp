#ifndef COMPAT_WINUTF8_HPP
#define COMPAT_WINUTF8_HPP
#include <Windows.h>

std::string lpwstr2string(LPWSTR wstr)
{
    auto cp = CP_UTF8;
    int cch = WideCharToMultiByte(cp, 0, wstr, -1, 0, 0, NULL, NULL);
    std::unique_ptr<char[]> psz(new char[cch]);
    memset(psz.get(), 0, cch);
    WideCharToMultiByte(cp, 0, wstr, -1, psz.get(), cch, NULL, NULL);
    return std::string(psz.get(), cch - 1);
}

#endif
