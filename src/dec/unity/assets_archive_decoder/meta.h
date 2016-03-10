#pragma once

#include "dec/unity/assets_archive_decoder/custom_stream.h"
#include "dec/unity/assets_archive_decoder/file_id_table.h"
#include "dec/unity/assets_archive_decoder/object_id_table.h"
#include "dec/unity/assets_archive_decoder/object_info_table.h"
#include "dec/unity/assets_archive_decoder/type_tree.h"
#include "dec/unity/assets_archive_decoder/util.h"

namespace au {
namespace dec {
namespace unity {

    struct Meta final
    {
        Meta(CustomStream &input_stream, const int version);

        // DataBlock type_tree_block;
        // DataBlock object_info_block;
        // DataBlock object_id_block;
        // DataBlock externals_block;

        std::unique_ptr<BaseTypeTree> type_tree;
        std::unique_ptr<BaseObjectInfoTable> object_info_table;
        std::unique_ptr<ObjectIdTable> object_id_table;
        std::unique_ptr<FileIdTable> file_id_table;

    };

} } }
