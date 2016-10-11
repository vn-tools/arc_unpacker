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

#include "dec/unity/assets_archive_decoder/meta.h"

using namespace au;
using namespace au::dec::unity;

template<typename TTypeTree, typename TType> static auto get_w_reader(
    CustomStream &input_stream)
{
    return std::make_unique<TTypeTree>(
        input_stream,
        [](CustomStream &input_stream)
        {
            return std::make_unique<TType>(input_stream);
        });
}

Meta::Meta(CustomStream &input_stream, const int version)
{
    // tree
    if (version > 13)
        type_tree = get_w_reader<TypeTreeV3, TypeV2>(input_stream);
    else if (version > 6)
        type_tree = get_w_reader<TypeTreeV2, TypeV1>(input_stream);
    else
        type_tree = get_w_reader<TypeTreeV1, TypeV1>(input_stream);

    // objects
    if (version > 14)
    {
        object_info_table
            = get_w_reader<ObjectInfoTableV2, ObjectInfoV3>(input_stream);
    }
    else if (version > 13)
    {
        object_info_table
            = get_w_reader<ObjectInfoTableV2, ObjectInfoV2>(input_stream);
    }
    else
    {
        object_info_table
            = get_w_reader<ObjectInfoTableV1, ObjectInfoV1>(input_stream);
    }

    // object ids
    if (version > 10)
        object_id_table = std::make_unique<ObjectIdTable>(input_stream);

    // file ids
    if (version > 5)
        file_id_table = get_w_reader<FileIdTable, FileIdV2>(input_stream);
    else
        file_id_table = get_w_reader<FileIdTable, FileIdV1>(input_stream);
}
