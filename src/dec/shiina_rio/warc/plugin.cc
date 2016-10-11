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

#include "dec/shiina_rio/warc/plugin.h"

using namespace au;
using namespace au::dec::shiina_rio;
using namespace au::dec::shiina_rio::warc;

void BaseExtraCrypt::decrypt(bstr &data, const u32 flags) const
{
    if (data.size() < min_size())
        return;
    if ((flags & 0x202) == 0x202)
        pre_decrypt(data);
    if ((flags & 0x204) == 0x204)
        post_decrypt(data);
}
