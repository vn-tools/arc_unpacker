#pragma once

#include "types.h"

namespace au {

    template<typename T> class ptr final
    {
    public:
        constexpr ptr(T *data, const size_t size) :
            start_ptr(data),
            cur_ptr(data),
            end_ptr(data + size)
        {
        }

        constexpr ptr &operator--()
        {
            cur_ptr -= sizeof(T);
            return *this;
        }

        constexpr ptr &operator++()
        {
            cur_ptr += sizeof(T);
            return *this;
        }

        constexpr ptr operator--(int)
        {
            auto p = *this;
            cur_ptr -= sizeof(T);
            return p;
        }

        constexpr ptr operator++(int)
        {
            auto p = *this;
            cur_ptr += sizeof(T);
            return p;
        }

        constexpr ptr<T> operator +(const size_t n) const
        {
            ptr ret(start_ptr, size());
            ret += n;
            return ret;
        }

        constexpr ptr<T> operator -(const size_t n) const
        {
            ptr ret(start_ptr, size());
            ret -= n;
            return ret;
        }

        ptr<T> operator +(const size_t n)
        {
            ptr ret(start_ptr, size());
            ret += n;
            return ret;
        }

        ptr<T> operator -(const size_t n)
        {
            ptr ret(start_ptr, size());
            ret -= n;
            return ret;
        }

        constexpr size_t size() const { return end_ptr - start_ptr; }
        constexpr void operator +=(const size_t n) const { cur_ptr += n; }
        constexpr void operator -=(const size_t n) const { cur_ptr -= n; }

        constexpr T &operator *() const { return *cur_ptr; }
        constexpr T &operator[](const size_t n) const { return cur_ptr[n]; }
        constexpr T *start() const { return start_ptr; }
        constexpr T *current() const { return cur_ptr; }
        constexpr T *end() const { return end_ptr; }

        void operator +=(const size_t n) { cur_ptr += n; }
        void operator -=(const size_t n) { cur_ptr -= n; }
        T &operator *() { return *cur_ptr; }
        T &operator[](const size_t n) { return cur_ptr[n]; }
        T *start() { return start_ptr; }
        T *current() { return cur_ptr; }
        T *end() { return end_ptr; }

        constexpr bool operator <(const T *p) const { return cur_ptr < p; }
        constexpr bool operator <=(const T *p) const { return cur_ptr <= p; }
        constexpr bool operator >(const T *p) const { return cur_ptr > p; }
        constexpr bool operator >=(const T *p) const { return cur_ptr >= p; }
        constexpr bool operator ==(const T *p) const { return cur_ptr == p; }
        constexpr bool operator !=(const T *p) const { return cur_ptr != p; }

    private:
        T *start_ptr;
        T *cur_ptr;
        T *end_ptr;
    };

    template<typename T> inline ptr<T> make_ptr(T *data, const size_t size)
    {
        return ptr<T>(data, size);
    }

    template<typename T> inline ptr<T> make_ptr(const ptr<T> &other)
    {
        return ptr<T>(other.current(), other.end() - other.current());
    }

    template<typename T> inline ptr<T> make_ptr(
        const ptr<T> &other, const size_t size)
    {
        return ptr<T>(other.current(), size);
    }

    inline ptr<u8> make_ptr(bstr &data)
    {
        return ptr<u8>(data.get<u8>(), data.size());
    }

    inline const ptr<const u8> make_ptr(const bstr &data)
    {
        return ptr<const u8>(data.get<const u8>(), data.size());
    }

}
