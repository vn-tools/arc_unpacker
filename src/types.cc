#include "types.h"
#include <algorithm>

using namespace au;

const size_t bstr::npos = static_cast<size_t>(-1);

bstr::bstr()
{
}

bstr::bstr(size_t n, u8 fill) : v(n, fill)
{
}

bstr::bstr(const u8 *str, size_t size) : v(str, str + size)
{
}

bstr::bstr(const char *str, size_t size) : v(str, str + size)
{
}

bstr::bstr(const std::string &other) : v(other.begin(), other.end())
{
}

std::string bstr::str(bool trim_to_zero) const
{
    if (trim_to_zero)
        return std::string(&v[0]);
    return std::string(&v[0], size());
}

bool bstr::empty() const
{
    return v.size() == 0;
}

size_t bstr::size() const
{
    return v.size();
}

size_t bstr::find(const bstr &other)
{
    auto pos = std::search(
        v.begin(), v.end(),
        other.get<char>(), other.get<char>() + other.size());
    if (pos == v.end())
        return bstr::npos;
    return pos - v.begin();
}

bstr bstr::substr(size_t start) const
{
    if (start > size())
        return ""_b;
    return bstr(get<const char>() + start, size() - start);
}

bstr bstr::substr(size_t start, size_t size) const
{
    if (start > v.size())
        return ""_b;
    if (size > v.size() || start + size > v.size())
        return substr(start, v.size() - start);
    return bstr(get<const char>() + start, size);
}

void bstr::resize(size_t how_much)
{
    v.resize(how_much);
}

void bstr::reserve(size_t how_much)
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
    v.push_back(c);
}

void bstr::operator +=(uint8_t c)
{
    v.push_back(c);
}

bool bstr::operator ==(const bstr &other) const
{
    return v == other.v;
}

bool bstr::operator !=(const bstr &other) const
{
    return v != other.v;
}

char &bstr::operator [](size_t pos)
{
    return v[pos];
}

const char &bstr::operator [](size_t pos) const
{
    return v[pos];
}
