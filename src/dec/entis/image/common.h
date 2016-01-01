#pragma once

#include "dec/entis/common/enums.h"
#include "types.h"

namespace au {
namespace dec {
namespace entis {
namespace image {

    enum EriImage
    {
        Rgb         = 0x00000001,
        Rgba        = 0x04000001,
        Gray        = 0x00000002,
        TypeMask    = 0x00FFFFFF,
        WithPalette = 0x01000000,
        UseClipping = 0x02000000,
        WithAlpha   = 0x04000000,
        SideBySide  = 0x10000000,
    };

    struct EriHeader final
    {
        u32 version;
        common::Transformation transformation;
        common::Architecture architecture;
        u32 format_type;
        u32 width;
        u32 height;
        bool flip;
        u32 bit_depth;
        u32 clipped_pixel;
        u32 sampling_flags;
        u32 quantumized_bits;
        u32 allotted_bits;
        u32 blocking_degree;
        u32 lapped_block;
        u32 frame_transform;
        u32 frame_degree;
    };

} } } }
