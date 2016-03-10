#pragma once

#include <functional>
#include <map>
#include "dec/unity/assets_archive_decoder/custom_stream.h"
#include "dec/unity/assets_archive_decoder/type.h"
#include "dec/unity/assets_archive_decoder/type_root.h"

namespace au {
namespace dec {
namespace unity {

    using TypeReader
        = std::function<std::unique_ptr<BaseType>(CustomStream &)>;

    struct BaseTypeTree : std::map<int, std::unique_ptr<TypeRoot>>
    {
        virtual ~BaseTypeTree() {}

        bool is_embedded;
    };

    struct TypeTreeV1 final : BaseTypeTree
    {
        TypeTreeV1(CustomStream &input_stream, const TypeReader);
    };

    struct TypeTreeV2 final : BaseTypeTree
    {
        TypeTreeV2(CustomStream &input_stream, const TypeReader);
    };

    struct TypeTreeV3 final : BaseTypeTree
    {
        TypeTreeV3(CustomStream &input_stream, const TypeReader);

        std::string revision;
        u32 attributes;
    };

} } }
