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

#include "dec/nitroplus/npk2_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::nitroplus;

Npk2ArchiveDecoder::Npk2ArchiveDecoder()
{
    plugin_manager.add(
        "tokyo-necro",
        "Tokyo Necro",
        "\x96\x2C\x5F\x3A\x78\x9C\x84\x37"
        "\xB7\x12\x12\xA1\x15\xD6\xCA\x9F"
        "\x9A\xE3\xFD\x21\x0F\xF6\xAF\x70"
        "\xA8\xA8\xF8\xBB\xFE\x5E\x8A\xF5"_b);

    plugin_manager.add(
        "sonicomi",
        "sonicomi",
        "\x65\xAB\xB4\xA8\xCD\xE0\xC8\x10"
        "\xBB\x4A\x26\x72\x37\x54\xC3\xA7"
        "\xE4\x3D\xE9\xEA\x7F\x5B\xB8\x43"
        "\x50\x1D\x05\xAB\xCF\x08\xD9\xC1"_b);

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects NPK2 decryption routine."));
}
