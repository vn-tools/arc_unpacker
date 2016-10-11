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
