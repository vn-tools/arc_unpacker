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

#include "dec/majiro/rct_image_decoder.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/majiro/rc8_image_decoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::majiro;

static const bstr magic = "\x98\x5A\x92\x9A\x54"_b; // sjis "六丁T"

static bstr decrypt(const bstr &input, const bstr &key)
{
    u32 crc_table[0x100];
    for (const auto i : algo::range(0x100))
    {
        u32 poly = i;
        for (const auto j : algo::range(8))
            poly = (poly >> 1) ^ ((poly & 1) ? 0xEDB88320 : 0);
        crc_table[i] = poly;
    }

    u32 checksum = 0xFFFFFFFF;
    for (const auto c : key)
        checksum = (checksum >> 8) ^ crc_table[(c ^ (checksum & 0xFF)) & 0xFF];
    checksum ^= 0xFFFFFFFF;

    bstr derived_key(0x400);
    for (const auto i : algo::range(0x100))
        derived_key.get<u32>()[i] = checksum ^ crc_table[(i + checksum) & 0xFF];

    bstr output(input);
    for (const auto i : algo::range(output.size()))
        output[i] ^= derived_key[i % derived_key.size()];
    return output;
}

static bstr uncompress(const bstr &input, size_t width, size_t height)
{
    io::MemoryByteStream input_stream(input);

    bstr output(width * height * 3);

    auto output_ptr = output.get<u8>();
    auto output_start = output.get<const u8>();
    auto output_end = output.end<const u8>();

    std::vector<int> shift_table;
    for (const auto i : algo::range(6))
        shift_table.push_back((-1 - i) * 3);
    for (const auto i : algo::range(7))
        shift_table.push_back((3 - i - width) * 3);
    for (const auto i : algo::range(7))
        shift_table.push_back((3 - i - width * 2) * 3);
    for (const auto i : algo::range(7))
        shift_table.push_back((3 - i - width * 3) * 3);
    for (const auto i : algo::range(5))
        shift_table.push_back((2 - i - width * 4) * 3);

    if (output.size() < 3)
        return output;
    *output_ptr++ = input_stream.read<u8>();
    *output_ptr++ = input_stream.read<u8>();
    *output_ptr++ = input_stream.read<u8>();

    while (output_ptr < output_end)
    {
        auto flag = input_stream.read<u8>();
        if (flag & 0x80)
        {
            auto size = flag & 3;
            auto look_behind = (flag >> 2) & 0x1F;
            size = size == 3
                ? (input_stream.read_le<u16>() + 4) * 3
                : size * 3 + 3;
            auto source_ptr = &output_ptr[shift_table[look_behind]];
            if (source_ptr < output_start || source_ptr + size >= output_end)
                return output;
            while (size-- && output_ptr < output_end)
                *output_ptr++ = *source_ptr++;
        }
        else
        {
            auto size = flag == 0x7F
                ? (input_stream.read_le<u16>() + 0x80) * 3
                : flag * 3 + 3;
            while (size-- && output_ptr < output_end)
                *output_ptr++ = input_stream.read<u8>();
        }
    }

    return output;
}

static std::unique_ptr<res::Image> try_decode(
    const Logger &logger, io::File &input_file)
{
    std::vector<std::shared_ptr<dec::BaseImageDecoder>> decoders;
    decoders.push_back(std::make_shared<RctImageDecoder>());
    decoders.push_back(std::make_shared<Rc8ImageDecoder>());

    for (const auto &decoder : decoders)
        if (decoder->is_recognized(input_file))
            return std::make_unique<res::Image>(
                decoder->decode(logger, input_file));
    return nullptr;
}

RctImageDecoder::RctImageDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--rct-key"})
                ->set_value_name("KEY")
                ->set_description("Decryption key (same for all files)");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_switch("rct-key"))
                key = algo::unhex(arg_parser.get_switch("rct-key"));
        });
}

bool RctImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image RctImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    bool encrypted;
    const auto tmp = input_file.stream.read<u8>();
    if (tmp == 'S')
        encrypted = true;
    else if (tmp == 'C')
        encrypted = false;
    else
        throw err::NotSupportedError("Unexpected encryption flag");

    const auto version = algo::from_string<int>(
        input_file.stream.read(2).str());
    if (version < 0 || version > 1)
        throw err::UnsupportedVersionError(version);

    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto data_size = input_file.stream.read_le<u32>();

    std::unique_ptr<res::Image> base_image;
    std::unique_ptr<res::Image> mask_image;
    if (version == 1)
    {
        const auto name_size = input_file.stream.read_le<u16>();
        const auto base_name
            = algo::trim_to_zero(input_file.stream.read(name_size).str());

        auto base_file = VirtualFileSystem::get_by_name(base_name);
        if (base_file)
            base_image = try_decode(logger, *base_file);
    }

    {
        auto mask_file = VirtualFileSystem::get_by_stem(
            input_file.path.stem() + "_");
        if (mask_file)
            mask_image = try_decode(logger, *mask_file);
    }

    auto data = input_file.stream.read(data_size);
    if (encrypted)
    {
        if (key.empty())
        {
            throw err::UsageError(
                "File is encrypted, but key not set. "
                "Please supply one with --rct-key switch.");
        }
        data = decrypt(data, key);
    }
    data = uncompress(data, width, height);

    res::Image overlay_image(width, height, data, res::PixelFormat::BGR888);
    if (version == 1)
    {
        for (const auto y : algo::range(height))
        for (const auto x : algo::range(width))
        {
            auto &p = overlay_image.at(x, y);
            if (p.r == 0xFF && p.g == 0 && p.g == 0)
                p.a = p.r = 0;
        }
    }

    res::Image output_image(width, height);
    if (base_image)
    {
        output_image.overlay(
            *base_image,
            res::Image::OverlayKind::OverwriteNonTransparent);
    }
    output_image.overlay(
        overlay_image, res::Image::OverlayKind::OverwriteNonTransparent);
    if (mask_image)
    {
        mask_image->invert();
        output_image.apply_mask(*mask_image);
    }
    return output_image;
}

static auto _ = dec::register_decoder<RctImageDecoder>("majiro/rct");
