#pragma once

#include "dec/unity/assets_archive_decoder/custom_stream.h"
#include "dec/unity/assets_archive_decoder/object_id.h"

namespace au {
namespace dec {
namespace unity {

    struct ObjectIdTable final : std::vector<std::unique_ptr<ObjectId>>
    {
        ObjectIdTable(CustomStream &custom_stream);
    };

} } }
