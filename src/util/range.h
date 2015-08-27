#pragma once

#include <algorithm>
#include <cstdio>

namespace au {
namespace util {

    struct RangeImpl
    {
        struct Iterator : std::iterator<std::random_access_iterator_tag,int,int>
        {
            int i, stride;

            constexpr Iterator(int i, int stride) : i(i), stride(stride)
            {
            }

            constexpr Iterator(Iterator it, int stride) : i(*it), stride(stride)
            {
            }

            constexpr int operator *()
            {
                return i;
            }

            Iterator operator ++()
            {
                i += stride;
                return *this;
            }

            constexpr bool operator !=(Iterator other)
            {
                return stride < 0 ? i > *other : i < *other;
            }
        };

        int stride;
        Iterator b, e;

        constexpr RangeImpl(int b, int e, int stride=1)
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
