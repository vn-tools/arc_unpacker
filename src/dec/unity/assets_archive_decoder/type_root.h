#pragma once

#include "dec/unity/assets_archive_decoder/custom_stream.h"
#include "dec/unity/assets_archive_decoder/node.h"
#include "dec/unity/assets_archive_decoder/util.h"

namespace au {
namespace dec {
namespace unity {

    struct TypeRoot final
    {
        int class_id;
        Hash script_id;
        Hash old_type_hash;
        std::unique_ptr<Node> root_node;
    };

} } }
