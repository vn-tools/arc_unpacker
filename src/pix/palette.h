#pragma once

#include <memory>
#include "io/io.h"
#include "pix/format.h"
#include "pix/pixel.h"

namespace au {
namespace pix {

    class Palette final
    {
    public:
        Palette(size_t color_count);
        Palette(size_t color_count, const bstr &input, Format format);
        Palette(size_t color_count, io::IO &input_io, Format format);
        ~Palette();

        size_t size() const;
        Pixel &operator [](size_t i);
        const Pixel &operator [](size_t i) const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
