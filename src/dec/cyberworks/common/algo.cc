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

#include "dec/cyberworks/common/algo.h"
#include "dec/cyberworks/dat_image_decoder.h"
#include "enc/png/png_image_encoder.h"

using namespace au;
using namespace au::dec::cyberworks;

static void decode_tinkerbell_data_headerless(bstr &data)
{
    const auto key =
        "DBB3206F-F171-4885-A131-EC7FBA6FF491 Copyright 2004 "
        "Cyberworks \"TinkerBell\"., all rights reserved.\x00"_b;
    size_t key_pos = 0;
    for (const auto i : algo::range(4, data.size()))
    {
        if (i >= 0xE1F)
            break;
        data[i] ^= key[key_pos];
        key_pos++;
        if (key_pos == key.size())
            key_pos = 1;
    }
    data[0] = 'O';
    data[1] = 'g';
    data[2] = 'g';
    data[3] = 'S';
}

static void decode_tinkerbell_data_with_header(bstr &data)
{
    data = data.substr(12);
    decode_tinkerbell_data_headerless(data);
}

u32 common::read_obfuscated_number(io::BaseByteStream &input_stream)
{
    u32 ret = 0;
    for (const auto i : algo::range(8))
    {
        ret *= 10;
        const auto byte = input_stream.read<u8>();
        if (byte != 0xFF)
            ret += byte ^ 0x7F;
    }
    return ret;
}

void common::decode_data(
    const Logger &logger,
    const bstr &type,
    bstr &data,
    const DatPlugin &plugin)
{
    if (type == "b0"_b || type == "n0"_b || type == "o0"_b)
    {
        io::File pseudo_file("dummy.dat", data);
        const auto image = DatImageDecoder(plugin).decode(
            logger, pseudo_file);
        data = enc::png::PngImageEncoder()
            .encode(logger, image, "")
                ->stream.seek(0).read_to_eof();
    }

    if (type == "j0"_b || type == "k0"_b)
        decode_tinkerbell_data_headerless(data);

    if (type == "u0"_b)
        decode_tinkerbell_data_with_header(data);

    if (type == "w0"_b)
        data = data.substr(5);
}
