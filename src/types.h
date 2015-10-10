#pragma once

#include <string>
#include <vector>

namespace au {

    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using s8 = int8_t;
    using s16 = int16_t;
    using s32 = int32_t;
    using s64 = int64_t;

    struct bstr final
    {
        static const std::size_t npos;

        bstr();
        bstr(size_t n, u8 fill = 0);
        bstr(const std::string &other);
        bstr(const u8 *str, size_t size);
        bstr(const char *str, size_t size);

        bool empty() const;
        std::size_t size() const;
        void resize(std::size_t how_much);
        void reserve(std::size_t how_much);

        std::size_t find(const bstr &other);
        bstr substr(std::size_t start) const;
        bstr substr(std::size_t start, std::size_t size) const;

        template<typename T> T *get()
        {
            return reinterpret_cast<T*>(&v[0]);
        }

        template<typename T> const T *get() const
        {
            return reinterpret_cast<const T*>(&v[0]);
        }

        template<typename T> T *end()
        {
            return get<T>() + v.size() / sizeof(T);
        }

        template<typename T> const T *end() const
        {
            return get<T>() + v.size() / sizeof(T);
        }

        u8 *begin()
        {
            return get<u8>();
        }

        u8 *end()
        {
            return end<u8>();
        }

        const u8 *begin() const
        {
            return get<const u8>();
        }

        const u8 *end() const
        {
            return end<const u8>();
        }

        std::string str(bool trim_to_zero = false) const;

        bstr operator +(const bstr &other);
        void operator +=(const bstr &other);
        void operator +=(char c);
        void operator +=(u8 c);
        bool operator !=(const bstr &other) const;
        bool operator ==(const bstr &other) const;
        char &operator [](std::size_t pos);
        const char &operator [](std::size_t pos) const;

    private:
        std::vector<char> v;
    };

    constexpr u8 operator "" _u8(char value)
    {
        return static_cast<u8>(value);
    }

    constexpr const u8* operator "" _u8(const char *value, size_t n)
    {
        return reinterpret_cast<const u8*>(value);
    }

    inline bstr operator "" _b(const char *value, size_t n)
    {
        return bstr(value, n);
    }

}
