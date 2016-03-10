#pragma once

#include "dec/unity/assets_archive_decoder/custom_stream.h"

namespace au {
namespace dec {
namespace unity {

    struct ObjectId final
    {
        ObjectId(CustomStream &input_stream);

        int serialized_file_index;
        long identifier_in_file;
    };

} } }
