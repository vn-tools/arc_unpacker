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
#include <vector>
#include "types.h"

namespace au {
namespace dec {
namespace cyberworks {

    struct DatPlugin final
    {
        std::map<std::string, std::vector<std::string>>
            toc_to_data_file_name_map;

        // parameters are in different order for each game
        u8 img_delim[3];
        std::vector<int> img_header_order;
        bool flip_img_vertically;
    };

} } }
