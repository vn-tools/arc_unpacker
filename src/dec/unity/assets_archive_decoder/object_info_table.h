// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

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
