#pragma once

#include <memory>
#include "io/io.h"
#include "pix/format.h"
#include "pix/palette.h"
#include "pix/pixel.h"

namespace au {
namespace pix {

    class Grid final
    {
    public:
        Grid(size_t width, size_t height);
        Grid(size_t width, size_t height, const bstr &input, Format fmt);
        Grid(size_t width, size_t height, io::IO &input_io, Format fmt);
        Grid(size_t width, size_t height, const bstr &input,
            const Palette &palette);
        Grid(size_t width, size_t height, io::IO &input_io,
            const Palette &palette);
        ~Grid();

        size_t width() const;
        size_t height() const;
        Pixel &at(size_t x, size_t y);
        const Pixel &at(size_t x, size_t y) const;

        void flip();
        void crop(size_t width, size_t height);

        Pixel *begin();
        Pixel *end();
        const Pixel *begin() const;
        const Pixel *end() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
