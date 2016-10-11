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

#include "dec/unity/assets_archive_decoder/type_tree.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::unity;

TypeTreeV1::TypeTreeV1(
    CustomStream &input_stream, const TypeReader type_reader)
{
    throw err::NotSupportedError("Type 1 trees are not implemented");
}

TypeTreeV2::TypeTreeV2(
    CustomStream &input_stream, const TypeReader type_reader)
{
    throw err::NotSupportedError("Type 2 trees are not implemented");
}

static std::unique_ptr<Node> read_node_v3(
    CustomStream &input_stream, const TypeReader &type_reader)
{
    throw err::NotSupportedError("Type 3 tree nodes are not implemented");
}

TypeTreeV3::TypeTreeV3(
    CustomStream &input_stream, const TypeReader type_reader)
{
    const auto revision = input_stream.read_to_zero();
    attributes = input_stream.read<u32>();

    is_embedded = input_stream.read<u8>() != 0;
    const auto num_base_classes = input_stream.read<u32>();
    for (const auto i : algo::range(num_base_classes))
    {
        auto type_root = std::make_unique<TypeRoot>();
        type_root->class_id = input_stream.read<s32>();
        if (type_root->class_id < 0)
            type_root->script_id = Hash(input_stream);
        type_root->old_type_hash = Hash(input_stream);
        if (is_embedded)
            type_root->root_node = read_node_v3(input_stream, type_reader);
        operator[](type_root->class_id) = std::move(type_root);
    }
}
