#pragma once

#include "dec/unity/assets_archive_decoder/custom_stream.h"
#include "dec/unity/assets_archive_decoder/util.h"

namespace au {
namespace dec {
namespace unity {

    struct BaseFileId
    {
        virtual ~BaseFileId() {}

        Hash guid;
        std::string path;
        int type;
    };

    struct FileIdV1 final : BaseFileId
    {
        FileIdV1(CustomStream &input_stream);
    };

    struct FileIdV2 final : BaseFileId
    {
        FileIdV2(CustomStream &input_stream);

        std::string asset_path;
    };

} } }
