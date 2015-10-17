#pragma once

#include <array>
#include "pix/grid.h"

namespace au {
namespace fmt {
namespace shiina_rio {
namespace warc {

    struct Plugin final
    {
        int version;
        int entry_name_size;

        bstr essential_crypt_key;
        std::array<u32, 5> initial_crypt_base_keys;

        bstr logo_data;
        std::shared_ptr<pix::Grid> region_image;

        struct
        {
            bstr table;
        } crc_crypt;

        struct
        {
            bstr table;
            bool pre1;
            bool pre2;
            bool post;
        } flag_crypt;
    };

} } } }
