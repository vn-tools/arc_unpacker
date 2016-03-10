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
