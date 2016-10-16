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

#include "dec/lucifen/lpk_archive_decoder.h"

using namespace au;
using namespace au::dec::lucifen;

LpkArchiveDecoder::LpkArchiveDecoder()
{
    plugin_manager.add(
        "sakura-sync",
        "Sakura Synchronicity",
        {
            {0xA5B9AC6B, 0x9A639DE5},
            0x5D,
            0x31746285,
            {
                {"script",  {0x00000000, 0x00000000}},
                {"sys",     {0xAE91B6B5, 0x9D42ED5C}},
                {"chr",     {0x7DC8994E, 0xB6E42499}},
                {"pic",     {0xC69D1636, 0x387DB369}},
                {"bgm",     {0x8C79B285, 0xBAE4AE69}},
                {"se",      {0x453B8E8B, 0x84C15E88}},
                {"voice",   {0x936E96DA, 0x5B7388E8}},
                {"data",    {0x2D4DAAC8, 0xE15D75AE}},
            }
        });

    plugin_manager.add(
        "happening-love",
        "Happening LOVE!!",
        {
            {0xA5B9AC6B, 0x9A639DE5},
            0x5D,
            0x31746285,
            {
                {"script",  {0x00000000, 0x00000000}},
                {"sys",     {0x69DBB6AD, 0x53D86E94}},
                {"chr",     {0x246739E6, 0x7C57A959}},
                {"pic",     {0xA4CB21A3, 0xC9B3A83D}},
                {"bgm",     {0x9C24DD6A, 0xDEE82BC6}},
                {"se",      {0x465DCEC5, 0xD349BE97}},
                {"voice",   {0x57C9E2E7, 0x365AC94B}},
                {"data",    {0x645DB9E6, 0xDB9B7536}},
            }
        });

    plugin_manager.add(
        "renai-harem",
        "Ren'ai Harem ~Daisuki tte Iwasete~",
        {
            {0xA5B9AC6B, 0x9A639DE5},
            0x5D,
            0x31746285,
            {
                {"script",  {0x00000000, 0x00000000}},
                {"sys",     {0x8EADF1AB, 0x4B234AC8}},
                {"chr",     {0xA65B783D, 0xF65E5A19}},
                {"pic",     {0x36AC5DBA, 0xD19741AD}},
                {"bgm",     {0x6BEA5B7E, 0x293F96B8}},
                {"se",      {0xC1464E68, 0xBA9E4C96}},
                {"voice",   {0x8435DCF3, 0xBEAD69E3}},
                {"data",    {0xDA75B679, 0xBAED5AC2}},
            }
        });

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects LPK decryption routine."));
}
