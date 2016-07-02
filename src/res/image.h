#pragma once

#include <memory>
#include "algo/grid.h"
#include "io/base_byte_stream.h"
#include "res/palette.h"
#include "res/pixel.h"
#include "res/pixel_format.h"

namespace au {
namespace res {

    class Image final : public algo::Grid<Pixel>
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
            io::BaseByteStream &input_stream,
            const PixelFormat fmt);

        Image(
            const size_t width,
            const size_t height,
            const bstr &input,
            const Palette &palette);

        Image(
            const size_t width,
            const size_t height,
            io::BaseByteStream &input_stream,
            const Palette &palette);

        Image &flip_vertically();
        Image &flip_horizontally();
        Image &offset(const int x, const int y);
        Image &crop(const size_t width, const size_t height);

        Image &invert();
        Image &apply_mask(const Image &other);
        Image &apply_palette(const Palette &palette);

        Image &overlay(
            const Image &other, const OverlayKind overlay_kind);
        Image &overlay(
            const Image &other,
            const int target_x,
            const int target_y,
            const OverlayKind overlay_kind);
    };

} }
