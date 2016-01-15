#pragma once

#include "err.h"
#include "types.h"

namespace au {
namespace algo {

    template<typename T> class BasePtr final
    {
    public:
        constexpr BasePtr(T *data, const size_t size) :
            start_ptr(data), cur_ptr(data), end_ptr(data + size)
        {
        }

        BasePtr &operator++()
        {
            cur_ptr++;
            return *this;
        }

        BasePtr &operator--()
        {
            cur_ptr--;
            return *this;
        }

        BasePtr operator++(int)
        {
            auto p = *this;
            operator++();
            return p;
        }

        BasePtr operator--(int)
        {
            auto p = *this;
            operator--();
            return p;
        }

        BasePtr &operator +=(const size_t n)
        {
            cur_ptr += n;
            return *this;
        }

        BasePtr &operator -=(const size_t n)
        {
            cur_ptr -= n;
            return *this;
        }

        constexpr BasePtr<T> operator +(const int n) const
        {
            BasePtr ret(start_ptr, size());
            ret += pos();
            ret += n;
            return ret;
        }

        constexpr BasePtr<T> operator -(const size_t n) const
        {
            return operator+(-n);
        }

        BasePtr<T> operator +(const int n)
        {
            BasePtr ret(start_ptr, size());
            ret += pos();
            ret += n;
            return ret;
        }

        BasePtr<T> operator -(const size_t n)
        {
            return operator+(-n);
        }

        void append_from(const bstr &source)
        {
            if (source.size() > left())
                throw err::BadDataSizeError();
            for (const auto &c : source)
                *cur_ptr++ = c;
        }

        void append_from(BasePtr<const T> &input_ptr, size_t size)
        {
            if (size > input_ptr.left())
                throw err::BadDataSizeError();
            if (size > left())
                throw err::BadDataSizeError();
            while (size--)
                *cur_ptr++ = *input_ptr++;
        }

        void append_from(const int relative_position, size_t size)
        {
            if (pos() + relative_position < 0)
                throw err::BadDataOffsetError();
            if (pos() + relative_position + size > this->size())
                throw err::BadDataOffsetError();
            if (size > left())
                throw err::BadDataSizeError();
            auto source_ptr = cur_ptr + relative_position;
            while (size--)
                *cur_ptr++ = *source_ptr++;
        }

        constexpr size_t pos() const { return cur_ptr - start_ptr; }
        constexpr size_t left() const { return end_ptr - cur_ptr; }
        constexpr size_t size() const { return end_ptr - start_ptr; }

        constexpr T &operator *() const { return *cur_ptr; }
        constexpr T &operator[](const size_t n) const { return cur_ptr[n]; }
        constexpr T *start() const { return start_ptr; }
        constexpr T *current() const { return cur_ptr; }
        constexpr T *end() const { return end_ptr; }

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


    template<typename T> using ptr = BasePtr<T>;


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

    inline ptr<const u8> make_ptr(const bstr &data)
    {
        return ptr<const u8>(data.get<const u8>(), data.size());
    }

} }
