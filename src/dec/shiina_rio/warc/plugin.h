#pragma once

#include <array>
#include <functional>
#include <memory>
#include "res/image.h"

namespace au {
namespace dec {
namespace shiina_rio {
namespace warc {

    struct BaseExtraCrypt
    {
        void decrypt(bstr &data, const u32 flags) const;

    protected:
        virtual size_t min_size() const = 0;
        virtual void pre_decrypt(bstr &data) const = 0;
        virtual void post_decrypt(bstr &data) const = 0;
    };

    struct Plugin final
    {
        int version;
        int entry_name_size;

        std::array<u32, 5> initial_crypt_base_keys;

        bstr logo_data;
        std::shared_ptr<res::Image> region_image;

        bstr crc_crypt_source;
        std::unique_ptr<BaseExtraCrypt> extra_crypt;
    };

    using PluginBuilder = std::function<std::shared_ptr<Plugin>()>;

} } } }
