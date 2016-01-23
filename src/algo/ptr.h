#pragma once

#include <limits>
#include "err.h"
#include "types.h"

namespace au {
namespace algo {

    template<typename T, bool cyclic> class BasePtr
    {
    public:
        constexpr BasePtr(T *data, const size_t size) :
            start_ptr(data), cur_ptr(data), end_ptr(data + size)
        {
        }

        inline T *operator->()
        {
            return cur_ptr;
        }

        inline const T *operator->() const
        {
            return cur_ptr;
        }

        inline BasePtr &operator++()
        {
            cur_ptr++;
            if (cyclic && cur_ptr == end_ptr)
                cur_ptr = start_ptr;
            return *this;
        }

        inline BasePtr &operator--()
        {
            if (cyclic && cur_ptr == start_ptr)
                cur_ptr = end_ptr - 1;
            else
                cur_ptr--;
            return *this;
        }

        inline BasePtr operator++(int)
        {
            auto p = *this;
            operator++();
            return p;
        }

        inline BasePtr operator--(int)
        {
            auto p = *this;
            operator--();
            return p;
        }

        inline BasePtr &operator +=(const size_t n)
        {
            cur_ptr += n;
            while (cyclic && cur_ptr >= end_ptr)
                cur_ptr -= size();
            return *this;
        }

        inline BasePtr &operator -=(const size_t n)
        {
            cur_ptr -= n;
            while (cyclic && cur_ptr < start_ptr)
                cur_ptr += size();
            return *this;
        }

        constexpr BasePtr operator +(const int n) const
        {
            BasePtr ret(start_ptr, size());
            ret += pos();
            ret += n;
            return ret;
        }

        constexpr BasePtr operator -(const size_t n) const
        {
            return operator+(-n);
        }

        inline BasePtr operator +(const int n)
        {
            BasePtr ret(start_ptr, size());
            ret += pos();
            ret += n;
            return ret;
        }

        inline BasePtr operator -(const size_t n)
        {
            return operator+(-n);
        }

        void append_from(const bstr &source)
        {
            if (!cyclic && source.size() > left())
                throw err::BadDataSizeError();
            for (const auto &c : source)
            {
                *cur_ptr = c;
                operator++();
            }
        }

        void append_from(BasePtr<const T, false> &input_ptr, size_t n)
        {
            if (n > input_ptr.left())
                throw err::BadDataSizeError();
            if (!cyclic && n > left())
                throw err::BadDataSizeError();
            while (n--)
            {
                *cur_ptr = *input_ptr++;
                operator++();
            }
        }

        void append_from(BasePtr<T, true> &input_ptr, size_t n)
        {
            if (n > input_ptr.left())
                throw err::BadDataSizeError();
            if (!cyclic && n > left())
                throw err::BadDataSizeError();
            while (n--)
            {
                *cur_ptr = *input_ptr++;
                operator++();
            }
        }

        void append_from(int relative_position, size_t n)
        {
            if (cyclic)
            {
                while (pos() + relative_position < 0)
                    relative_position += size();
                while (pos() + relative_position >= size())
                    relative_position -= size();
            }
            else
            {
                if (pos() + relative_position < 0)
                    throw err::BadDataOffsetError();
                if (pos() + relative_position + n > size())
                    throw err::BadDataOffsetError();
                if (n > left())
                    throw err::BadDataSizeError();
            }
            BasePtr<T, cyclic> source_ptr(start_ptr, size());
            source_ptr += pos() + relative_position;
            while (n--)
            {
                *cur_ptr = *source_ptr++;
                operator++();
            }
        }

        constexpr size_t pos() const
        {
            return cur_ptr - start_ptr;
        }

        inline size_t left() const
        {
            if (cyclic)
                return std::numeric_limits<size_t>::max();
            return end_ptr - cur_ptr;
        }

        constexpr size_t size() const
        {
            return end_ptr - start_ptr;
        }

        T &operator *()
        {
            return *cur_ptr;
        }

        constexpr T &operator *() const
        {
            return *cur_ptr;
        }

        T &operator[](size_t n)
        {
            return start_ptr[(pos() + n) % size()];
        }

        constexpr T &operator[](size_t n) const
        {
            return start_ptr[(pos() + n) % size()];
        }

        T *start() { return start_ptr; }
        constexpr T *start() const { return start_ptr; }

        T *current() { return cur_ptr; }
        constexpr T *current() const { return cur_ptr; }

        T *end() { return end_ptr; }
        constexpr T *end() const { return end_ptr; }

    private:
        T *start_ptr;
        T *cur_ptr;
        T *end_ptr;
    };

    template<typename T> using ptr = BasePtr<T, false>;
    template<typename T> using cyclic_ptr = BasePtr<T, true>;


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


    template<typename T> inline cyclic_ptr<T> make_cyclic_ptr(
        T *data, const size_t size)
    {
        return cyclic_ptr<T>(data, size);
    }

    inline cyclic_ptr<u8> make_cyclic_ptr(bstr &data)
    {
        return cyclic_ptr<u8>(data.get<u8>(), data.size());
    }

    inline cyclic_ptr<const u8> make_cyclic_ptr(const bstr &data)
    {
        return cyclic_ptr<const u8>(data.get<const u8>(), data.size());
    }

} }
