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
#include <memory>
#include "types.h"

namespace au {
namespace dec {
namespace purple_software {
namespace cpz5 {

    struct Plugin final
    {
        bstr secret;

        std::array<size_t, 4> hash_permutation;
        std::array<u32, 4> hash_iv;
        std::array<u32, 4> hash_xor;
        std::array<u32, 4> hash_add;

        u32 crypt_23_mul;

        struct
        {
            u32 add1;
            u32 add2;
            u8 tail_sub;
        } crypt_1a;

        struct
        {
            std::array<u32, 4> addends;
            u32 init_key;
        } crypt_1c;

        struct
        {
            size_t start_pos;
            u8 tail_xor;
        } crypt_3;
    };

    std::vector<std::shared_ptr<cpz5::Plugin>> get_cpz5_plugins();
    std::vector<std::shared_ptr<cpz5::Plugin>> get_cpz6_plugins();

} } } }
