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

#include "dec/kid/cps_file_decoder.h"
#include "dec/kid/lnd_file_decoder.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::kid;

static const bstr magic = "CPS\x00"_b;

static bstr decrypt(
    const bstr &input,
    const size_t size_comp,
    const uoff_t offset)
{
    io::MemoryByteStream input_stream(input);
    io::MemoryByteStream output_stream;
    output_stream.reserve(input_stream.size());

    const auto real_offset = offset - 16;
    input_stream.seek(real_offset);
    u32 key = input_stream.read_le<u32>() + offset + 0x3786425;

    input_stream.seek(0);
    while (input_stream.left())
    {
        const bool use_key = input_stream.pos() != real_offset;
        auto value = input_stream.read_le<u32>();
        if (use_key)
        {
            value -= size_comp;
            value -= key;
        }
        output_stream.write_le<u32>(value);
        key = key * 0x41C64E6D + 0x9B06;
    }
    output_stream.write_le<u32>(0);

    output_stream.seek(4);
    return output_stream.read_to_eof();
}

bool CpsFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> CpsFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    const auto size_comp = input_file.stream.read_le<u32>();
    const auto version = input_file.stream.read_le<u16>();
    const auto compression_type = input_file.stream.read_le<u16>();
    const auto size_orig = input_file.stream.read_le<u32>();

    auto data = input_file.stream.read(size_comp - 16 - 4);
    const auto offset = input_file.stream.read_le<u32>() - 0x7534682;
    if (offset)
        data = decrypt(data, size_comp, offset);

    if (compression_type & 1)
        data = LndFileDecoder::decompress_raw_data(data, size_orig);

    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("prt");
    return output_file;
}

static auto _ = dec::register_decoder<CpsFileDecoder>("kid/cps");
