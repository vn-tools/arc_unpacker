#pragma once

#include <map>
#include "dec/unity/assets_archive_decoder/custom_stream.h"
#include "dec/unity/assets_archive_decoder/object_info.h"

namespace au {
namespace dec {
namespace unity {

    using ObjectInfoReader
        = std::function<std::unique_ptr<BaseObjectInfo>(CustomStream &)>;

    struct BaseObjectInfoTable : std::map<s64, std::unique_ptr<BaseObjectInfo>>
    {
        virtual ~BaseObjectInfoTable() {}
    };

    struct ObjectInfoTableV1 final : BaseObjectInfoTable
    {
        ObjectInfoTableV1(CustomStream &custom_stream, const ObjectInfoReader);
    };

    struct ObjectInfoTableV2 final : BaseObjectInfoTable
    {
        ObjectInfoTableV2(CustomStream &custom_stream, const ObjectInfoReader);
    };

} } }
