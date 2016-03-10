#pragma once

#include "dec/unity/assets_archive_decoder/custom_stream.h"

namespace au {
namespace dec {
namespace unity {

    struct BaseObjectInfo
    {
        virtual ~BaseObjectInfo() {}
        bool is_script() const;

        uoff_t offset;
        uoff_t size;
        int type_id;
        int class_id;
    };

    struct ObjectInfoV1 final : BaseObjectInfo
    {
        ObjectInfoV1(CustomStream &input_stream);

        short is_destroyed;
    };

    struct ObjectInfoV2 final : BaseObjectInfo
    {
        ObjectInfoV2(CustomStream &input_stream);

        short script_type_index;
    };

    struct ObjectInfoV3 final : BaseObjectInfo
    {
        ObjectInfoV3(CustomStream &input_stream);

        short script_type_index;
        bool is_stripped;
    };

} } }
