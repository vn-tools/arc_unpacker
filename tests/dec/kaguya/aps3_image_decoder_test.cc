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
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

static bstr compress(const bstr &input)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.min_match_size = 2;
    settings.position_bits = 12;
    settings.size_bits = 4;
    settings.initial_dictionary_pos = 1;
    return algo::pack::lzss_compress(input, settings);
}

static bstr encode_to_ap(const res::Image &input_image)
{
    io::MemoryByteStream output_stream;
    output_stream.write("AP");
    output_stream.write_le<u32>(input_image.width());
    output_stream.write_le<u32>(input_image.height());
    output_stream.write_le<u16>(24);
    for (const auto y : algo::range(input_image.height() - 1, -1, -1))
    for (const auto x : algo::range(input_image.width()))
    {
        output_stream.write<u8>(input_image.at(x, y).b);
        output_stream.write<u8>(input_image.at(x, y).g);
        output_stream.write<u8>(input_image.at(x, y).r);
        output_stream.write<u8>(input_image.at(x, y).a);
    }
    return output_stream.seek(0).read_to_eof();
}

TEST_CASE("Kaguya APS3 images", "[dec]")
{
    const auto decoder = Aps3ImageDecoder();
    const auto input_image = tests::get_transparent_test_image();
    const auto data = encode_to_ap(input_image);

    const std::vector<std::string> dummy_entries = {"123.png", "456.png"};

    io::File input_file;
    input_file.stream.write("\x04""APS3"_b);
    input_file.stream.write_le<u32>(dummy_entries.size());
    for (const auto i : algo::range(dummy_entries.size()))
    {
        const auto &entry = dummy_entries[i];
        input_file.stream.write_le<u32>(i);
        input_file.stream.write<u8>(entry.size());
        input_file.stream.write(entry);
        // probably coordinates - didn't bother to figure them out,
        // as the image data is conveniently encoded as a whole anyway
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
    }

    SECTION("Uncompressed")
    {
        input_file.stream.write_le<u32>(6 + data.size());
        input_file.stream.write_le<u16>(0);
        input_file.stream.write_le<u32>(data.size());
        input_file.stream.write(data);
    }

    SECTION("Compressed")
    {
        const auto data_comp = compress(data);
        input_file.stream.write_le<u32>(10 + data_comp.size());
        input_file.stream.write_le<u16>(1);
        input_file.stream.write_le<u32>(data_comp.size());
        input_file.stream.write_le<u32>(data.size());
        input_file.stream.write(data_comp);
    }

    const auto expected_image = input_image;
    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(actual_image, expected_image);
}
