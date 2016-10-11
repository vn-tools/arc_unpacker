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

#include "dec/unity/assets_archive_decoder/file_id.h"

using namespace au;
using namespace au::dec::unity;

FileIdV1::FileIdV1(CustomStream &input_stream)
{
    guid = Hash(input_stream),
    type = input_stream.read<u32>();
    path = input_stream.read_to_zero().str();
}

FileIdV2::FileIdV2(CustomStream &input_stream)
{
    asset_path = input_stream.read_to_zero().str();
    guid = Hash(input_stream),
    type = input_stream.read<u32>();
    path = input_stream.read_to_zero().str();
}
