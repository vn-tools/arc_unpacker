#pragma once

#include <map>
#include "dec/unity/assets_archive_decoder/custom_stream.h"
#include "dec/unity/assets_archive_decoder/file_id.h"

namespace au {
namespace dec {
namespace unity {

    using FileIdReader
        = std::function<std::unique_ptr<BaseFileId>(CustomStream &)>;

    struct FileIdTable final : std::vector<std::unique_ptr<BaseFileId>>
    {
        FileIdTable(CustomStream &custom_stream, const FileIdReader);
    };

} } }
