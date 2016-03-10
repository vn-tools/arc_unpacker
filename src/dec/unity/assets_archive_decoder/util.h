#pragma once

#include <array>
#include "dec/unity/assets_archive_decoder/custom_stream.h"

namespace au {
namespace dec {
namespace unity {

    struct DataBlock final
    {
        uoff_t offset;
        uoff_t size;
    };

    struct Hash final : std::array<u8, 16>
    {
        Hash();
        Hash(CustomStream &input_stream);
    };

} } }
