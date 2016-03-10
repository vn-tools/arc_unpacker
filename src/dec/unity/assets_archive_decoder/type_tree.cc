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
