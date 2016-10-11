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

#include "dec/cronus/grp_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/cronus/common.h"
#include "err.h"
#include "plugin_manager.h"

using namespace au;
using namespace au::dec::cronus;

namespace
{
    enum class EncryptionType : u8
    {
        Delta,
        SwapBytes,
        None,
    };

    struct Header final
    {
        size_t width;
        size_t height;
        size_t bpp;
        uoff_t input_offset;
        size_t output_size;
        bool flip;
        bool use_transparency;
        EncryptionType encryption_type;
    };

    using HeaderReader = std::function<
        std::unique_ptr<Header>(io::BaseByteStream&)>;
}

static void swap_decrypt(bstr &input, size_t encrypted_size)
{
    u16 *input_ptr = input.get<u16>();
    const u16 *input_end = input.end<const u16>();
    size_t repetitions = encrypted_size >> 1;
    while (repetitions-- && input_ptr < input_end)
    {
        *input_ptr = ((*input_ptr << 8) | (*input_ptr >> 8)) ^ 0x33CC;
        input_ptr++;
    }
}

struct GrpImageDecoder::Priv final
{
    PluginManager<HeaderReader> plugin_manager;
};

static bool validate_header(const Header &header)
{
    size_t expected_output_size = header.width * header.height;
    if (header.bpp == 8)
        expected_output_size += 1024;
    else if (header.bpp == 24)
        expected_output_size *= 3;
    else if (header.bpp == 32)
        expected_output_size *= 4;
    else
        return false;
    return header.output_size == expected_output_size;
}

static HeaderReader get_v1_reader(
    const u32 key1,
    const u32 key2,
    const u32 key3,
    const EncryptionType enc_type)
{
    return [=](io::BaseByteStream &input_stream)
    {
        auto header = std::make_unique<Header>();
        header->width = input_stream.read_le<u32>() ^ key1;
        header->height = input_stream.read_le<u32>() ^ key2;
        header->bpp = input_stream.read_le<u32>();
        input_stream.skip(4);
        header->output_size = input_stream.read_le<u32>() ^ key3;
        input_stream.skip(4);
        header->use_transparency = false;
        header->flip = true;
        header->encryption_type = enc_type;
        return header;
    };
}

static HeaderReader get_v2_reader()
{
    return [](io::BaseByteStream &input_stream)
    {
        auto header = std::make_unique<Header>();
        input_stream.skip(4);
        header->output_size = input_stream.read_le<u32>();
        input_stream.skip(8);
        header->width = input_stream.read_le<u32>();
        header->height = input_stream.read_le<u32>();
        header->bpp = input_stream.read_le<u32>();
        input_stream.skip(8);
        header->flip = false;
        header->use_transparency = input_stream.read_le<u32>() != 0;
        header->encryption_type = EncryptionType::None;
        return header;
    };
}

GrpImageDecoder::GrpImageDecoder() : p(new Priv)
{
    p->plugin_manager.add(
        "dokidoki", "Doki Doki Princess",
        get_v1_reader(
            0xA53CC35A, 0x35421005, 0xCF42355D, EncryptionType::SwapBytes));

    p->plugin_manager.add(
        "sweet", "Sweet Pleasure",
        get_v1_reader(
            0x2468FCDA, 0x4FC2CC4D, 0xCF42355D, EncryptionType::Delta));

    p->plugin_manager.add("nursery", "Nursery Song", get_v2_reader());
}

GrpImageDecoder::~GrpImageDecoder()
{
}

static std::unique_ptr<Header> read_header(
    const PluginManager<HeaderReader> &plugin_manager, io::File &input_file)
{
    for (const auto header_func : plugin_manager.get_all())
    {
        input_file.stream.seek(0);
        try
        {
            auto header = header_func(input_file.stream);
            if (!header)
                continue;
            if (!validate_header(*header))
                continue;
            header->input_offset = input_file.stream.pos();
            return header;
        }
        catch (...)
        {
            continue;
        }
    }
    return nullptr;
}

bool GrpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return read_header(p->plugin_manager, input_file) != nullptr;
}

res::Image GrpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto header = read_header(p->plugin_manager, input_file);
    input_file.stream.seek(header->input_offset);
    auto data = input_file.stream.read_to_eof();

    if (header->encryption_type == EncryptionType::Delta)
        delta_decrypt(data, get_delta_key(input_file.path.name()));
    else if (header->encryption_type == EncryptionType::SwapBytes)
        swap_decrypt(data, header->output_size);
    data = algo::pack::lzss_decompress(data, header->output_size);

    std::unique_ptr<res::Image> image;

    if (header->bpp == 8)
    {
        res::Palette palette(256, data, res::PixelFormat::BGRA8888);
        image = std::make_unique<res::Image>(
            header->width, header->height, data.substr(1024), palette);
    }
    else if (header->bpp == 24)
    {
        image = std::make_unique<res::Image>(
            header->width, header->height, data, res::PixelFormat::BGR888);
    }
    else if (header->bpp == 32)
    {
        image = std::make_unique<res::Image>(
            header->width,
            header->height,
            data,
            res::PixelFormat::BGRA8888);
    }
    else
    {
        throw err::UnsupportedBitDepthError(header->bpp);
    }

    if (!header->use_transparency)
    {
        for (const auto x : algo::range(header->width))
        for (const auto y : algo::range(header->height))
            image->at(x, y).a = 0xFF;
    }
    if (header->flip)
        image->flip_vertically();

    return *image;
}

static auto _ = dec::register_decoder<GrpImageDecoder>("cronus/grp");
