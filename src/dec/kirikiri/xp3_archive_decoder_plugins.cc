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

#include "dec/kirikiri/xp3_archive_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "dec/kirikiri/cxdec.h"
#include "io/program_path.h"

using namespace au;
using namespace au::dec::kirikiri;

static bstr read_etc_file(const std::string &name)
{
    const auto path = io::get_assets_dir_path() / "xp3" / name;
    io::File file(path, io::FileMode::Read);
    return file.stream.seek(0).read_to_eof();
}

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
        "xor-p1-neg", "XOR variation",
        create_simple_plugin([](bstr &data, u32 key)
        {
            for (const auto i : algo::range(0, data.size()))
                data[i] ^= (key + 1) ^ 0xFF;
        }));

    plugin_manager.add(
        "xor-mix", "XOR variation",
        create_simple_plugin([](bstr &data, u32 key)
        {
            for (const auto i : algo::range(0, data.size(), 2))
                data[i] ^= key;
            for (const auto i : algo::range(1, data.size(), 2))
                data[i] ^= i;
        }));

    plugin_manager.add(
        "dieselmine", "Games from Dieselmine",
        create_simple_plugin([](bstr &data, u32 key)
        {
            auto data_ptr = algo::make_ptr(data);
            while (data_ptr.left() && data_ptr.pos() < 0x7B)
                *data_ptr++ ^= 21 * key;
            while (data_ptr.left() && data_ptr.pos() < 0xF6)
                *data_ptr++ -= 32 * key;
            while (data_ptr.left() && data_ptr.pos() < 0x171)
                *data_ptr++ ^= 43 * key;
            while (data_ptr.left())
                *data_ptr++ -= 54 * key;
        }));

    plugin_manager.add(
        "rebirth", "Re:birth colony ~Lost azurite~",
        create_simple_plugin([](bstr &data, u32 key)
        {
            for (const auto i : algo::range(5, data.size()))
                data[i] ^= (key >> 12);
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
        "fha", "Fate/Hollow Ataraxia",
        create_cxdec_plugin(
            0x143, 0x787, {0,1,2}, {0,1,2,3,4,5,6,7}, {0,1,2,3,4,5}));

    plugin_manager.add(
        "comyu", "Comyu - Kuroi Ryuu to Yasashii Oukoku",
        create_cxdec_plugin(
            0x1A3, 0x0B6, {0,1,2}, {0,7,5,6,3,1,4,2}, {4,3,2,1,5,0}));

    plugin_manager.add(
        "mahoyoru", "Mahou Tsukai no Yoru",
        create_cxdec_plugin(
            0x22A, 0x2A2, {1,0,2}, {7,6,5,1,0,3,4,2}, {3,2,1,4,5,0}));

    plugin_manager.add(
        "natsuzora", "Natsuzora Kanata",
        create_cxdec_plugin(
            0x2F5, 0x6F0, {2,0,1}, {7,2,3,6,1,0,5,4}, {2,3,4,0,1,5}));

    plugin_manager.add(
        "tenshin", "Tenshin Ranman - Lucky or Unlucky!?",
        create_cxdec_plugin(
            0x167, 0x498, {1,0,2}, {4,2,3,5,6,1,7,0}, {1,0,5,4,3,2}));

    plugin_manager.add(
        "dracuriot", "Dracu-Riot!",
        create_cxdec_plugin(
            0x2F0, 0x418, {2,0,1}, {5,3,0,2,1,4,6,7}, {0,3,5,4,2,1}));

    plugin_manager.add(
        "lavender", "Kourin no Machi, Lavender no Shoujo",
        create_cxdec_plugin(
            0x181, 0x635, {2,1,0}, {7,5,2,3,6,1,4,0}, {4,0,1,5,2,3}));

    plugin_manager.add(
        "karakara", "Karakara",
        create_cxdec_plugin(
            0x190, 0x4A7, {1,0,2}, {2,0,7,3,5,1,4,6}, {2,1,0,5,4,3},
            read_etc_file("karakara.dat")));

    plugin_manager.add(
        "waremete", "Ushinawareta Mirai o Motomete",
        create_cxdec_plugin(
            0x23C, 0x60F, {2,0,1}, {1,5,0,3,2,7,6,4}, {4,5,2,1,0,3}));

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects XP3 decryption routine."));
}
