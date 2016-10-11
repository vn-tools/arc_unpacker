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
