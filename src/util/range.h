#ifndef AU_UTIL_RANGE_H
#define AU_UTIL_RANGE_H
#include <algorithm>

namespace au {
namespace util {

    struct RangeImpl
    {
        struct Iterator
            : std::iterator<std::random_access_iterator_tag,int,int>
        {
            int i;
            int stride;

            constexpr Iterator(int i, int stride)
                : i(i), stride(stride)
            {
            }

            constexpr Iterator(Iterator it, int stride)
                : i(*it), stride(stride)
            {
            }

            constexpr int operator*()
            {
                return i;
            }

            Iterator operator++()
            {
                ++i;
                return *this;
            }

            Iterator operator++(int)
            {
                auto cpy = *this;
                ++(*this);
                return cpy;
            }

            constexpr Iterator operator+(int n)
            {
                return Iterator((i + n) / stride, stride);
            }

            constexpr Iterator operator-(int n)
            {
                return Iterator((i - n) / stride, stride);
            }

            constexpr int operator-(Iterator other)
            {
                return(i - *other) / stride;
            }

            constexpr bool operator==(Iterator other)
            {
                return i == *other;
            }

            constexpr bool operator!=(Iterator other)
            {
                return i != *other;
            }

            Iterator& operator+=(int other)
            {
                i += other * stride;
                return *this;
            }
        };

        int stride;
        Iterator b, e;

        constexpr RangeImpl(int b, int e, int stride=1)
            : stride(stride), b(b,stride), e(e,stride)
        {
        }

        constexpr RangeImpl(Iterator b, Iterator e, int stride=1)
            : stride(stride), b(b,stride), e(e,stride)
        {
        }

        constexpr Iterator begin()
        {
            return b;
        }

        constexpr Iterator end()
        {
            return e;
        }
    };

    constexpr RangeImpl range(int b, int e, int stride=1)
    {
        return RangeImpl(b, e, stride);
    }

    constexpr RangeImpl range(int e)
    {
        return RangeImpl(0, e);
    }

} }

#endif
