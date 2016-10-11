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

#include "dec/purple_software/cpz5/plugin.h"

using namespace au;
using namespace au::dec::purple_software;
using namespace au::dec::purple_software::cpz5;

static std::shared_ptr<Plugin> get_5_base_plugin()
{
    auto p = std::make_shared<Plugin>();
    p->crypt_1a.add1 = 0x784C5962;
    p->crypt_1a.add2 = 0x01010101;
    p->crypt_1a.tail_sub = 0x79;
    p->crypt_1c.addends = {0, 0x112233, 0, 0x34258765};
    p->crypt_1c.init_key = 0x2A65CB4E;
    p->crypt_3.start_pos = 9;
    return p;
}

static std::shared_ptr<Plugin> get_5v1_plugin()
{
    auto p = get_5_base_plugin();
    p->secret =
        "\x89\xF0\x90\xCD\x82\xB7\x82\xE9\x88\xAB\x82\xA2\x8E\x71\x82\xCD"
        "\x83\x8A\x83\x52\x82\xAA\x82\xA8\x8E\x64\x92\x75\x82\xAB\x82\xB5"
        "\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x81\x42\x8E\xF4\x82\xED"
        "\x82\xEA\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x82\xE6\x81\x60"
        "\x81\x41\x82\xC6\x82\xA2\x82\xA4\x82\xA9\x82\xE0\x82\xA4\x8E\xF4"
        "\x82\xC1\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB5\x82\xBD\x81\xF4"_b;
    p->hash_permutation = {3, 1, 2, 0};
    p->hash_iv  = {0xC74A2B01, 0xE7C8AB8F, 0xD8BEDC4E, 0x7302A4C5};
    p->hash_xor = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    p->hash_add = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    p->crypt_3.tail_xor = 0xBC;
    p->crypt_23_mul = 0x1A743125;
    return p;
}

static std::shared_ptr<Plugin> get_5v2_plugin()
{
    auto p = get_5_base_plugin();
    p->secret =
        "\x89\xF0\x90\xCD\x82\xB7\x82\xE9\x88\xAB\x82\xA2\x8E\x71\x82\xCD"
        "\x83\x8A\x83\x52\x82\xAA\x82\xA8\x8E\x64\x92\x75\x82\xAB\x82\xB5"
        "\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x81\x42\x83\x81\x83\x62"
        "\x81\x49\x82\xA9\x82\xED\x82\xA2\x82\xA2\x82\xC6\x82\xA9\x8C\xBE"
        "\x82\xC1\x82\xC4\x82\xE0\x8B\x96\x82\xB5\x82\xC4\x82\xA0\x82\xB0"
        "\x82\xC8\x82\xA2\x82\xF1\x82\xBE\x82\xA9\x82\xE7\x82\x9F\x81\x49"_b;
    p->hash_permutation = {1, 2, 3, 0};
    p->hash_iv  = {0x53FE9B2C, 0xF2C93EA8, 0xEE81BA59, 0xA2C8973E};
    p->hash_xor = {0x49875325, 0x00000000, 0xAD7948B7, 0x00000000};
    p->hash_add = {0x00000000, 0x54F46D7D, 0x00000000, 0x1D0638AD};
    p->crypt_3.tail_xor = 0xCB;
    p->crypt_23_mul = 0x1A740235;
    return p;
}

static std::shared_ptr<Plugin> get_6_plugin()
{
    auto p = std::make_shared<Plugin>();
    p->secret =
        "\x89\xF0\x90\xCD\x82\xB7\x82\xE9\x88\xAB\x82\xA2\x8E\x71\x82\xCD"
        "\x83\x8A\x83\x52\x82\xAA\x82\xA8\x8E\x64\x92\x75\x82\xAB\x82\xB5"
        "\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x81\x42\x8E\xF4\x82\xED"
        "\x82\xEA\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x82\xE6\x81\x60"
        "\x81\x41\x82\xC6\x82\xA2\x82\xA4\x82\xA9\x82\xE0\x82\xA4\x8E\xF4"
        "\x82\xC1\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB5\x82\xBD\x81\xF4"_b;
    p->hash_permutation = {2, 1, 0, 3};
    p->hash_iv  = {0xC74A2B01, 0xE7C8AB8F, 0xD8BEDC4E, 0x7302A4C5};
    p->hash_xor = {0x45A76C2F, 0x00000000, 0x79ABE8AD, 0x00000000};
    p->hash_add = {0x00000000, 0xA45E8035, 0x00000000, 0xE3F7A9E5};
    p->crypt_1a.add1 = 0x784C5062;
    p->crypt_1a.add2 = 0x01010101;
    p->crypt_1a.tail_sub = 0x7D;
    p->crypt_1c.addends = {0, 0x11003322, 0, 0x34216785};
    p->crypt_1c.init_key = 0x2A65CB4F;
    p->crypt_3.start_pos = 10;
    p->crypt_3.tail_xor = 0xAE;
    p->crypt_23_mul = 0x1A74F195;
    return p;
}

std::vector<std::shared_ptr<Plugin>> cpz5::get_cpz5_plugins()
{
    std::vector<std::shared_ptr<Plugin>> plugins;
    plugins.push_back(std::move(get_5v1_plugin()));
    plugins.push_back(std::move(get_5v2_plugin()));
    return plugins;
}

std::vector<std::shared_ptr<Plugin>> cpz5::get_cpz6_plugins()
{
    std::vector<std::shared_ptr<Plugin>> plugins;
    plugins.push_back(std::move(get_6_plugin()));
    return plugins;
}
