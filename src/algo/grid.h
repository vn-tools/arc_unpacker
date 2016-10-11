// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include "algo/range.h"
#include "err.h"

namespace au {
namespace algo {

    template<typename T> class Grid
    {
    public:
        Grid(const size_t width, const size_t height)
            : content(width * height), _width(width), _height(height)
        {
            if (!width || !height)
                throw err::BadDataSizeError();
        }

        Grid(const Grid &other) : Grid(other.width(), other.height())
        {
            for (const auto y : algo::range(_height))
            for (const auto x : algo::range(_width))
                at(x, y) = other.at(x, y);
        }

        virtual ~Grid()
        {
        }

        size_t width() const
        {
            return _width;
        }

        size_t height() const
        {
            return _height;
        }

        T &at(const size_t x, const size_t y)
        {
            return content[x + y * _width];
        }

        const T &at(const size_t x, const size_t y) const
        {
            return content[x + y * _width];
        }

        T *begin()
        {
            return content.data();
        }

        T *end()
        {
            return content.empty() ? nullptr : begin() + _width * _height;
        }

        const T *begin() const
        {
            return content.data();
        }

        const T *end() const
        {
            return content.empty() ? nullptr : begin() + _width * _height;
        }

    protected:
        std::vector<T> content;
        size_t _width, _height;
    };

} }
