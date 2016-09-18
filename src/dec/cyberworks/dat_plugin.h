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
