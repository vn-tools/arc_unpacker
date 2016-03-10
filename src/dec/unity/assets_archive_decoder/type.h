#pragma once

#include "dec/unity/assets_archive_decoder/custom_stream.h"

namespace au {
namespace dec {
namespace unity {

    struct BaseType
    {
        virtual ~BaseType() {}

        std::string type;
        std::string name;
        int size;
        int index;
        bool is_array;
        u32 version;
        u32 meta_flag;
    };

    struct TypeV1 final : BaseType
    {
        TypeV1(CustomStream &input_stream);
    };

    struct TypeV2 final : BaseType
    {
        TypeV2(CustomStream &input_stream);

        int tree_level;
        uoff_t type_offset;
        uoff_t name_offset;
    };

} } }
