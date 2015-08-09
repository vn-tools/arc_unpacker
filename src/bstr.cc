#include <algorithm>
#include "bstr.h"

const std::size_t bstr::npos = static_cast<std::size_t>(-1);

bstr::bstr()
{
}

bstr::bstr(const char *str, std::size_t size) : v(str, str + size)
{
}

bstr::bstr(const std::string &other) : v(other.begin(), other.end())
{
}

std::string bstr::str() const
{
    return std::string(&v[0], size());
}

std::size_t bstr::size() const
{
    return v.size();
}

std::size_t bstr::find(const bstr &other)
{
    auto pos = std::search(
        v.begin(), v.end(),
        other.get<char>(), other.get<char>() + other.size());
    if (pos == v.end())
        return bstr::npos;
    return pos - v.begin();
}

bstr bstr::substr(std::size_t start) const
{
    return bstr(get<const char>() + start, size() - start);
}

bstr bstr::substr(std::size_t start, std::size_t size) const
{
    return bstr(get<const char>() + start, size);
}

void bstr::resize(std::size_t how_much)
{
    v.resize(how_much);
}

void bstr::reserve(std::size_t how_much)
{
    v.reserve(how_much);
}

bstr bstr::operator +(const bstr &other)
{
    bstr ret(&v[0], size());
    ret += other;
    return ret;
}

void bstr::operator +=(const bstr &other)
{
    v.insert(v.end(), other.get<char>(), other.get<char>() + other.size());
}

void bstr::operator +=(char c)
{
    v.insert(v.end(), c);
}

void bstr::operator +=(uint8_t c)
{
    v.insert(v.end(), c);
}

bool bstr::operator ==(const bstr &other) const
{
    return v == other.v;
}

bool bstr::operator !=(const bstr &other) const
{
    return v != other.v;
}

char &bstr::operator [](std::size_t pos)
{
    return v[pos];
}

const char &bstr::operator [](std::size_t pos) const
{
    return v[pos];
}
