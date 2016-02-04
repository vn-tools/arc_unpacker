#include "dec/kirikiri/xp3_archive_decoder.h"
#include "algo/range.h"
#include "dec/kirikiri/cxdec.h"

using namespace au;
using namespace au::dec::kirikiri;

static Xp3Plugin create_simple_plugin(const Xp3DecryptFunc &xp3_decrypt_func)
{
    Xp3Plugin plugin;
    plugin.create_decrypt_func
        = [=](const io::path &arc_name) -> Xp3DecryptFunc
        {
            return xp3_decrypt_func;
        };
    return plugin;
}

Xp3ArchiveDecoder::Xp3ArchiveDecoder()
{
    plugin_manager.add(
        "noop", "Unecrypted games",
        create_simple_plugin([](bstr &data, u32 key) { }));

    plugin_manager.add(
        "xor", "Basic XOR encryption",
        create_simple_plugin([](bstr &data, u32 key)
        {
            for (const auto i : algo::range(data.size()))
                data[i] ^= key;
        }));

    plugin_manager.add(
        "fsn", "Fate/Stay Night",
        create_simple_plugin([](bstr &data, u32 key)
        {
            for (const auto i : algo::range(data.size()))
                data[i] ^= 0x36;
            if (data.size() > 0x2EA29)
                data[0x2EA29] ^= 3;
            if (data.size() > 0x13)
                data[0x13] ^= 1;
        }));

    plugin_manager.add(
        "rebirth", "Re:birth colony ~Lost azurite~",
        create_simple_plugin([](bstr &data, u32 key)
        {
            for (const auto i : algo::range(5, data.size()))
                data[i] ^= (key >> 12);
        }));

    plugin_manager.add(
        "mixed-xor", "Gokkun! Onii-chan Milk ~Punipuni Oppai na Imouto to~",
        create_simple_plugin([](bstr &data, u32 key)
        {
            for (const auto i : algo::range(0, data.size(), 2))
                data[i] ^= key;
            for (const auto i : algo::range(1, data.size(), 2))
                data[i] ^= i;
        }));

    plugin_manager.add(
        "orcsoft", "Oku-sama wa Moto Yariman -Besluted-",
        create_simple_plugin([](bstr &data, u32 key)
        {
            for (const auto i : algo::range(0, data.size()))
                data[i] ^= (key + 1) ^ 0xFF;
        }));

    plugin_manager.add(
        "comyu", "Comyu - Kuroi Ryuu to Yasashii Oukoku",
        create_cxdec_plugin(
            0x1A3, 0x0B6, {0,1,2}, {0,7,5,6,3,1,4,2}, {4,3,2,1,5,0}));

    plugin_manager.add(
        "fha", "Fate/Hollow Ataraxia",
        create_cxdec_plugin(
            0x143, 0x787, {0,1,2}, {0,1,2,3,4,5,6,7}, {0,1,2,3,4,5}));

    plugin_manager.add(
        "mahoyoru", "Mahou Tsukai no Yoru",
        create_cxdec_plugin(
            0x22A, 0x2A2, {1,0,2}, {7,6,5,1,0,3,4,2}, {3,2,1,4,5,0}));

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects XP3 decryption routine."));
}
