#include "dec/unity/assets_archive_decoder/object_info.h"

using namespace au;
using namespace au::dec::unity;

bool BaseObjectInfo::is_script() const
{
    return type_id < 0;
}

ObjectInfoV1::ObjectInfoV1(CustomStream &input_stream)
{
    offset = input_stream.read<u32>();
    size = input_stream.read<u32>();
    type_id = input_stream.read<s32>();
    class_id = input_stream.read<s16>();
    is_destroyed = input_stream.read<s16>();
}

ObjectInfoV2::ObjectInfoV2(CustomStream &input_stream)
{
    offset = input_stream.read<u32>();
    size = input_stream.read<u32>();
    type_id = input_stream.read<s32>();
    class_id = input_stream.read<s16>();
    script_type_index = input_stream.read<s16>();
}

ObjectInfoV3::ObjectInfoV3(CustomStream &input_stream)
{
    offset = input_stream.read<u32>();
    size = input_stream.read<u32>();
    type_id = input_stream.read<s32>();
    class_id = input_stream.read<s16>();
    script_type_index = input_stream.read<s16>();
    is_stripped = input_stream.read<u8>() != 0;
}
