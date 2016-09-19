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
}
