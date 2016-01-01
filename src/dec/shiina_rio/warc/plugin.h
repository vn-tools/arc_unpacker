#pragma once

#include <array>
#include "res/image.h"

namespace au {
namespace dec {
namespace shiina_rio {
namespace warc {

    using CrcCryptFunc = std::function<void(bstr &data)>;
    using FlagCryptFunc = std::function<void(bstr &data, const u32 flags)>;

    struct Plugin final
    {
        int version;
        int entry_name_size;

        std::array<u32, 5> initial_crypt_base_keys;

        bstr logo_data;
        std::shared_ptr<res::Image> region_image;

        CrcCryptFunc crc_crypt;
        FlagCryptFunc flag_pre_crypt;
        FlagCryptFunc flag_post_crypt;
    };

} } } }
