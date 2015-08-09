#ifndef AU_BSTR_H
#define AU_BSTR_H
#include <vector>
#include <string>

struct bstr
{
    static const std::size_t npos;

    bstr();
    bstr(const std::string &other);
    bstr(const char *str, size_t size);

    std::size_t size() const;
    void resize(std::size_t how_much);
    void reserve(std::size_t how_much);

    std::size_t find(const bstr &other);
    bstr substr(std::size_t start) const;
    bstr substr(std::size_t start, std::size_t length) const;

    template<typename T> T *get()
    {
        return reinterpret_cast<T*>(&v[0]);
    }

    template<typename T> const T *get() const
    {
        return reinterpret_cast<const T*>(&v[0]);
    }

    template<typename T> T &get(std::size_t pos)
    {
        return reinterpret_cast<T&>(v[pos]);
    }

    template<typename T> const T &get(std::size_t pos) const
    {
        return reinterpret_cast<const T&>(v[pos]);
    }

    std::string str() const;

    bstr operator +(const bstr &other);
    void operator +=(char c);
    void operator +=(uint8_t c);
    void operator +=(const bstr &other);
    bool operator !=(const bstr &other) const;
    bool operator ==(const bstr &other) const;
    char &operator [](std::size_t pos);
    const char &operator [](std::size_t pos) const;

private:
    std::vector<char> v;
};

#endif
