#pragma once

#include <array>
#include "res/image.h"

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
        std::shared_ptr<res::Image> region_image;

        struct
        {
            bstr table;
        } crc_crypt;

        struct
        {
            bstr table;
            std::function<void(const Plugin &, bstr &, const u32)> pre;
            std::function<void(const Plugin &, bstr &, const u32)> post;
        } flag_crypt;
    };

} } } }
