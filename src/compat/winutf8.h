#ifndef COMPAT_WINUTF8_H
#define COMPAT_WINUTF8_H
#include <string>

std::wstring string2wstring(std::string str);
std::string wstring2string(std::wstring wstr);

#endif
