#ifndef COMPAT_WINUTF8_HPP
#define COMPAT_WINUTF8_HPP
#ifdef _WIN32

    #include <Windows.h>
    #include <memory>
    #include <string>
    #include "compat/winutf8.h"

    std::wstring string2wstring(std::string str)
    {
        auto cp = CP_UTF8;
        int cch = MultiByteToWideChar(cp, 0, str.c_str(), -1, 0, 0);
        std::unique_ptr<wchar_t[]> psz(new wchar_t[cch]);
        memset(psz.get(), 0, cch);
        MultiByteToWideChar(cp, 0, str.c_str(), -1, psz.get(), cch);
        return std::wstring(psz.get(), cch - 1);
    }

    std::string wstring2string(std::wstring wstr)
    {
        auto cp = CP_UTF8;
        int cch = WideCharToMultiByte(
            cp, 0, wstr.c_str(), -1, 0, 0, nullptr, nullptr);
        std::unique_ptr<char[]> psz(new char[cch]);
        memset(psz.get(), 0, cch);
        WideCharToMultiByte(
            cp, 0, wstr.c_str(), -1, psz.get(), cch, nullptr, nullptr);
        return std::string(psz.get(), cch - 1);
    }

#endif
#endif
