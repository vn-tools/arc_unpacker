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
