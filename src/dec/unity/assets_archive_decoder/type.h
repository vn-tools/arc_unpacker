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
