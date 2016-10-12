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

#include "dec/cyberworks/common/plugins.h"

using namespace au;
using namespace au::dec::cyberworks;

void common::register_plugins(PluginManager<DatPlugin> &plugin_manager)
{
    const auto _ = -1;

    plugin_manager.add(
        "aniyome-kyouka",
        "Aniyome Kyouka-san to Sono Haha Chikako-san "
            "~Bijin Tsuma to Bijukubo to Issho~",
        {
            {
                {"dPih.dat", {"dPi.dat"}},
                {"dSch.dat", {"dSc.dat"}},
                {"dSo.dat", {"dSoh.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {5, 7, 0, 6, 4, 3, _},
            true,
        });

    plugin_manager.add(
        "zoku-etsuraku",
        "Zoku Etsuraku no Tane",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat", "Arc05a.dat", "Arc05b.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
                {"Arc07.dat", {"Arc08.dat"}},
                {"Arc09.dat", {"Arc10.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, 5, _, _, _, _, 6, _, _, 3, _, _, _, 0, _, _, _, 7, _, _},
            false,
        });

    plugin_manager.add(
        "shukubo-no-uzuki",
        "Shukubo no Uzuki ~Hitozuma Miboujin no Nareta Karada to Amai Toiki~",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat", "Arc05a.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, _, _, 3, _, _, _, _, _, _, _, 5, _, 0, _, _, _, 7, _, _},
            false,
        });

    plugin_manager.add(
        "shukubo-no-uzuki2",
        "Shukubo no Uzuki 2 ~Nareta Hitozuma kara Tadayou \"Onna\" no Iroka~",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat", "Arc05a.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, 5, _, _, _, _, _, _, 3, _, _, _, _, 0, _, _, _, 7, _, _},
            false,
        });

    plugin_manager.add(
        "cosplay-ecchi",
        "Cosplay Ecchi ~Layer Kana no Yuuutsu~",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, _, _, _, 3, _, _, _, _, 6, 5, 0, _, _, _, _, _, 7, _, _},
            false,
        });

    plugin_manager.add(
        "ouma-no-shoku",
        "Ouma no Shoku ~Sei ni Tsukaeshi Yami no Guuzou~",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat", "Arc05a.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, _, _, _, _, 5, _, _, _, _, 3, _, _, 0, _, _, _, 7, _, _},
            false,
        });

    plugin_manager.add(
        "inyou-goku",
        "Inyouchuu Goku ~Ryoushoku Jigoku Taimaroku~",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat", "Arc05a.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, 5, _, _, _, _, _, _, _, _, 3, _, _, 0, _, _, _, 7, _, _},
            false,
        });

    plugin_manager.add(
        "inyou-rei-1",
        "In'youchuu Rei ~Ryoujoku Shiro Taima Emaki~ (Miyuki Hen)",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, 5, _, _, _, _, _, _, _, _, _, 3, _, 0, _, _, _, 7, _, _},
            false,
        });

    plugin_manager.add(
        "inyou-rei-2",
        "In'youchuu Rei ~Ryoujoku Shiro Taima Emaki~ (Yui Hen)",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, 5, _, _, _, _, _, _, _, _, 3, _, _, 0, _, _, _, 7, _, _},
            false,
        });
}
