#pragma once

#include <memory>
#include "io/istream.h"
#include "res/palette.h"
#include "res/pixel.h"
#include "res/pixel_format.h"

namespace au {
namespace res {

    class Image final
    {
    public:
        enum class OverlayKind : u8
        {
            OverwriteAll,
            OverwriteNonTransparent,
            AddSimple,
        };

        Image(const Image &other);

        Image(const size_t width, const size_t height);

        Image(
            const size_t width,
            const size_t height,
            const bstr &input,
            const PixelFormat fmt);

        Image(
            const size_t width,
            const size_t height,
            io::IStream &input_stream,
            const PixelFormat fmt);

        Image(
            const size_t width,
            const size_t height,
            const bstr &input,
            const Palette &palette);

        Image(
            const size_t width,
            const size_t height,
            io::IStream &input_stream,
            const Palette &palette);

        ~Image();

        size_t width() const
        {
            return _width;
        }

        size_t height() const
        {
            return _height;
        }

        Pixel &at(const size_t x, const size_t y)
        {
            return pixels[x + y * _width];
        }

        const Pixel &at(const size_t x, const size_t y) const
        {
            return pixels[x + y * _width];
        }

        void flip_vertically();
        void flip_horizontally();
        void crop(const size_t width, const size_t height);
        void apply_mask(const Image &other);
        void apply_palette(const Palette &palette);

        void overlay(
            const Image &other, const OverlayKind overlay_kind);
        void overlay(
            const Image &other,
            const int target_x,
            const int target_y,
            const OverlayKind overlay_kind);

        Pixel *begin();
        Pixel *end();
        const Pixel *begin() const;
        const Pixel *end() const;

    private:
        std::vector<Pixel> pixels;
        size_t _width, _height;
    };

} }
