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

#include <memory>
#include "io/base_byte_stream.h"
#include "res/pixel.h"
#include "res/pixel_format.h"

namespace au {
namespace res {

    class Palette final
    {
    public:
        Palette(const Palette &other);

        Palette(const size_t color_count);

        Palette(
            const size_t color_count,
            const bstr &input,
            const PixelFormat format);

        Palette(
            const size_t color_count,
            io::BaseByteStream &input_stream,
            const PixelFormat format);

        ~Palette();

        size_t size() const;
        Pixel &at(const size_t i);
        const Pixel &at(const size_t i) const;
        Pixel &operator [](const size_t i);
        const Pixel &operator [](const size_t i) const;

        Pixel *begin();
        Pixel *end();
        const Pixel *begin() const;
        const Pixel *end() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
