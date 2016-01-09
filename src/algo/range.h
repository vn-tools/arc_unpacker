#pragma once

#include <algorithm>

namespace au {
namespace algo {

    struct Range final
    {
        struct Iterator final
            : std::iterator<std::random_access_iterator_tag, int, int>
        {
            int i, stride;

            Iterator(int i, int stride) : i(i), stride(stride)
            {
            }

            Iterator(Iterator it, int stride) : i(*it), stride(stride)
            {
            }

            int operator *() const
            {
                return i;
            }

            Iterator operator ++()
            {
                i += stride;
                return *this;
            }

            bool operator !=(Iterator other) const
            {
                return stride < 0 ? i > *other : i < *other;
            }
        };

        Range(int b, int e, int stride = 1)
            : stride(stride), b(b, stride), e(e, stride)
        {
        }

        Iterator begin() const
        {
            return b;
        }

        Iterator end() const
        {
            return e;
        }

        int stride;
        Iterator b, e;
    };

    inline Range range(int b, int e, int stride=1)
    {
        return Range(b, e, stride);
    }

    inline Range range(int e)
    {
        return Range(0, e);
    }

} }
