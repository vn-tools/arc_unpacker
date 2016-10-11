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

#include "dec/kaguya/aps3_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/kaguya/ao_image_decoder.h"
#include "dec/kaguya/ap_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "\x04""APS3"_b;

static bstr decompress(const bstr &input, const size_t size_orig)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.min_match_size = 2;
    settings.position_bits = 12;
    settings.size_bits = 4;
    settings.initial_dictionary_pos = 1;
    return algo::pack::lzss_decompress(input, size_orig, settings);
}

bool Aps3ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image Aps3ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());

    const auto entry_count = input_file.stream.read_le<u32>();
    for (const auto i : algo::range(entry_count))
    {
        const auto index = input_file.stream.read_le<u32>();
        const auto name_size = input_file.stream.read<u8>();
        const auto name = input_file.stream.read_to_zero(name_size).str();
        input_file.stream.skip(7 * 4);
    }

    const auto remaining_size = input_file.stream.read_le<u32>();
    if (remaining_size != input_file.stream.left())
        throw err::BadDataSizeError();

    const auto is_compressed = input_file.stream.read_le<u16>() == 1;
    bstr data;
    if (is_compressed)
    {
        const auto size_comp = input_file.stream.read_le<u32>();
        const auto size_orig = input_file.stream.read_le<u32>();
        data = decompress(input_file.stream.read(size_comp), size_orig);
    }
    else
    {
        const auto size = input_file.stream.read_le<u32>();
        data = input_file.stream.read(size);
    }

    std::vector<std::unique_ptr<dec::BaseImageDecoder>> decoders;
    decoders.push_back(std::make_unique<ApImageDecoder>());
    decoders.push_back(std::make_unique<AoImageDecoder>());

    io::File pseudo_file("dummy.dat", data);
    for (const auto &decoder : decoders)
        if (decoder->is_recognized(pseudo_file))
            return decoder->decode(logger, pseudo_file);

    throw err::RecognitionError();
}

static auto _ = dec::register_decoder<Aps3ImageDecoder>("kaguya/aps3");
