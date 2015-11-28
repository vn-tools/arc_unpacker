#pragma once

#include <array>
#include "res/image.h"

namespace au {
namespace fmt {
namespace shiina_rio {
namespace warc {

    struct Plugin;

    using FlagCryptFunc = std::function<void(
        const Plugin &plugin, bstr &data, const u32 flags)>;

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
            FlagCryptFunc pre;
            FlagCryptFunc post;
        } flag_crypt;
    };

} } } }
