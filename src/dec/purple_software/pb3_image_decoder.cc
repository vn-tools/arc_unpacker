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

#include "dec/purple_software/pb3_image_decoder.h"
#include <array>
#include "algo/binary.h"
#include "algo/format.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/png/png_image_decoder.h"
#include "dec/purple_software/jbp1.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::purple_software;

static const bstr magic = "PB3B"_b;

namespace
{
    struct Header final
    {
        int main_type;
        int sub_type;
        size_t width;
        size_t height;
        size_t depth;
    };
}

static bstr custom_lzss_decompress(
    const bstr &control_block,
    const bstr &data_block,
    const size_t output_size)
{
    std::array<u8, 0x800> dict = {0};
    auto dict_ptr = algo::make_cyclic_ptr(dict.data(), dict.size()) + 0x7DE;
    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);
    io::MemoryByteStream control_block_stream(control_block);
    io::MemoryByteStream data_block_stream(data_block);
    int control = 0, bit_mask = 0;
    while (output_ptr.left())
    {
        if (!bit_mask)
        {
            bit_mask = 0x80;
            control = control_block_stream.read<u8>();
        }
        if (control & bit_mask)
        {
            const auto tmp = data_block_stream.read_le<u16>();
            const auto look_behind_pos = tmp >> 5;
            auto repetitions = (tmp & 0x1F) + 3;
            auto source_ptr
                = algo::make_cyclic_ptr(dict.data(), dict.size())
                + look_behind_pos;
            while (repetitions-- && output_ptr.left())
            {
                const auto b = *source_ptr++;
                *output_ptr++ = b;
                *dict_ptr++ = b;
            }
        }
        else
        {
            const auto b = data_block_stream.read<u8>();;
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        bit_mask >>= 1;
    }
    return output;
}

static res::Image unpack_v1(
    const Header &header, io::BaseByteStream &input_stream)
{
    const auto channel_count = header.depth >> 3;
    const auto stride = header.width * channel_count;
    res::Image output_image(header.width, header.height);

    const auto main_sizes_offset = input_stream.seek(0x2C).read_le<u32>();
    const auto data_sizes_offset = input_stream.seek(0x30).read_le<u32>();

    input_stream.seek(main_sizes_offset);
    std::vector<size_t> main_sizes;
    for (const auto channel : algo::range(channel_count))
        main_sizes.push_back(input_stream.read_le<u32>());

    input_stream.seek(data_sizes_offset);
    std::vector<size_t> data_sizes;
    for (const auto channel : algo::range(channel_count))
        data_sizes.push_back(input_stream.read_le<u32>());

    std::vector<uoff_t> main_offsets;
    std::vector<uoff_t> data_offsets;
    main_offsets.push_back(main_sizes_offset + 4 * channel_count);
    data_offsets.push_back(data_sizes_offset + 4 * channel_count);
    for (const auto channel : algo::range(1, channel_count))
    {
        main_offsets.push_back(main_offsets.back() + main_sizes[channel - 1]);
        data_offsets.push_back(data_offsets.back() + data_sizes[channel - 1]);
    }

    for (const auto channel : algo::range(channel_count))
    {
        input_stream.seek(main_offsets[channel]);
        const auto control_block1_size = input_stream.read_le<u32>();
        const auto data_block1_size = input_stream.read_le<u32>();
        const auto size_orig = input_stream.read_le<u32>();

        const auto control_block1 = input_stream.read(control_block1_size);
        const auto data_block1 = input_stream.read(data_block1_size);
        const auto control_block2 = input_stream.read(
            main_offsets[channel] + main_sizes[channel] - input_stream.pos());

        const auto data_block2 = input_stream
            .seek(data_offsets[channel])
            .read(data_sizes[channel]);

        const auto plane = custom_lzss_decompress(
            control_block2, data_block2, size_orig);

        const size_t block_size = 16;
        size_t x_block_count = header.width / block_size;
        size_t y_block_count = header.height / block_size;
        if (header.width % block_size) x_block_count++;
        if (header.height % block_size) y_block_count++;

        io::MemoryByteStream control_block1_stream(control_block1);
        io::MemoryByteStream data_block1_stream(data_block1);
        io::MemoryByteStream plane_stream(plane);
        int bit_mask = 0, control = 0;
        for (const auto block_y : algo::range(y_block_count))
        for (const auto block_x : algo::range(x_block_count))
        {
            const size_t block_x1 = block_x * block_size;
            const size_t block_y1 = block_y * block_size;
            const size_t block_x2
                = std::min(block_x1 + block_size, header.width);
            const size_t block_y2
                = std::min(block_y1 + block_size, header.height);

            if (!bit_mask)
            {
                control = control_block1_stream.read<u8>();
                bit_mask = 0x80;
            }
            if (control & bit_mask)
            {
                const auto b = data_block1_stream.read<u8>();
                for (const auto y : algo::range(block_y1, block_y2))
                for (const auto x : algo::range(block_x1, block_x2))
                    output_image.at(x, y)[channel] = b;
            }
            else
            {
                for (const auto y : algo::range(block_y1, block_y2))
                for (const auto x : algo::range(block_x1, block_x2))
                    output_image.at(x, y)[channel] = plane_stream.read<u8>();
            }
            bit_mask >>= 1;
        }
    }

    if (header.depth != 32)
        for (auto &c : output_image)
            c.a = 0xFF;
    return output_image;
}

static res::Image unpack_v2(
    const Header &header, io::BaseByteStream &input_stream)
{
    input_stream.seek(0x2C);

    const auto mask_data_offset = input_stream.read_le<u32>();
    const auto mask_data_size = input_stream.read_le<u32>();
    const auto jbp1_data_size = mask_data_offset - input_stream.pos();
    const auto jbp1_data = input_stream.read(jbp1_data_size);
    const auto mask_data = input_stream
        .seek(mask_data_offset).read(mask_data_size);

    res::Image output_image(
        header.width,
        header.height,
        jbp1_decompress(jbp1_data),
        res::PixelFormat::BGR888);

    bstr final_mask_data;
    final_mask_data.reserve(header.width * header.height);
    io::MemoryByteStream mask_stream(mask_data);
    while (mask_stream.left())
    {
        const auto control = mask_stream.read<u8>();
        if (control == 0 || control == 0xFF)
            final_mask_data += bstr(mask_stream.read<u8>(), control);
        else
            final_mask_data += control;
    }

    res::Image mask_image(
        header.width, header.height, final_mask_data, res::PixelFormat::Gray8);

    output_image.apply_mask(mask_image);
    return output_image;
}

static res::Image unpack_v3(
    const Header &header, io::BaseByteStream &input_stream)
{
    const auto jbp1_data = input_stream.seek(0x34).read_to_eof();
    return res::Image(
        header.width,
        header.height,
        jbp1_decompress(jbp1_data),
        res::PixelFormat::BGR888);
}

static res::Image unpack_v5(
    const Header &header, io::BaseByteStream &input_stream)
{
    const auto channel_count = header.depth >> 3;
    const auto stride = header.width * channel_count;
    res::Image output_image(header.width, header.height);

    input_stream.seek(0x34);
    std::vector<uoff_t> control_offsets;
    std::vector<uoff_t> data_offsets;
    for (const auto i : algo::range(channel_count))
    {
        control_offsets.push_back(0x54 + input_stream.read_le<u32>());
        data_offsets.push_back(0x54 + input_stream.read_le<u32>());
    }

    std::vector<size_t> control_sizes;
    std::vector<size_t> data_sizes;
    for (const auto i : algo::range(1, channel_count))
    {
        control_sizes.push_back(control_offsets[i] - control_offsets[i - 1]);
        data_sizes.push_back(data_offsets[i] - data_offsets[i - 1]);
    }
    control_sizes.push_back(input_stream.size() - control_offsets.back());
    data_sizes.push_back(input_stream.size() - data_offsets.back());

    for (const auto channel : algo::range(channel_count))
    {
        const auto control_block = input_stream
            .seek(control_offsets[channel])
            .read(control_sizes[channel]);
        const auto data_block = input_stream
            .seek(data_offsets[channel])
            .read(data_sizes[channel]);

        const auto plane = custom_lzss_decompress(
            control_block, data_block, header.width * header.height);
        auto plane_ptr = algo::make_ptr(plane);

        u8 acc = 0;
        for (const auto y : algo::range(header.height))
        for (const auto x : algo::range(header.width))
        {
            acc += *plane_ptr++;
            output_image.at(x, y)[channel] = acc;
        }
    }
    return output_image;
}

static res::Image unpack_v6(
    const Logger &logger,
    const Header &header,
    io::BaseByteStream &input_stream)
{
    static const auto name_key =
        "\xA6\x75\xF3\x9C\xC5\x69\x78\xA3"
        "\x3E\xA5\x4F\x79\x59\xFE\x3A\xC7"_b;

    auto base_stem = algo::trim_to_zero(
        algo::unxor(input_stream.seek(0x34).read(0x20), name_key)).str();

    res::Image output_image(header.width, header.height);

    const auto base_file = VirtualFileSystem::get_by_stem(base_stem);
    if (base_file)
    {
        const std::vector<std::shared_ptr<dec::BaseImageDecoder>> decoders
            {
                std::make_shared<Pb3ImageDecoder>(),
                std::make_shared<dec::png::PngImageDecoder>()
            };
        for (const auto &decoder : decoders)
        {
            if (decoder->is_recognized(*base_file))
            {
                const auto base_image = decoder->decode(logger, *base_file);
                output_image.overlay(
                    base_image, res::Image::OverlayKind::OverwriteAll);
            }
        }
    }

    const auto size_orig = input_stream.seek(0x18).read_le<u32>();

    const auto control_block_offset
        = 0x20 + input_stream.seek(0x0C).read_le<u32>();
    const auto data_block_offset
        = control_block_offset + input_stream.seek(0x2C).read_le<u32>();
    const auto data_block_size = input_stream.seek(0x30).read_le<u32>();
    const auto control_block_size = data_block_offset - control_block_offset;

    const auto control_block1 = input_stream
        .seek(control_block_offset)
        .read(control_block_size);

    const auto data_block1 = input_stream
        .seek(data_block_offset)
        .read(data_block_size);

    const auto proxy_block = custom_lzss_decompress(
        control_block1, data_block1, size_orig);

    io::MemoryByteStream proxy_block_stream(proxy_block);
    const auto control_block2_size = proxy_block_stream.read_le<u32>();
    const auto data_block2_size = proxy_block_stream.read_le<u32>();
    const auto control_block2 = proxy_block_stream.read(control_block2_size);
    const auto data_block2 = proxy_block_stream.read(data_block2_size);

    io::MemoryByteStream control_block2_stream(control_block2);
    io::MemoryByteStream data_block2_stream(data_block2);

    const auto block_size = 8;
    auto x_block_count = header.width / block_size;
    auto y_block_count = header.height / block_size;
    if (header.width % block_size) x_block_count++;
    if (header.height % block_size) y_block_count++;

    int bit_mask = 0, control = 0;
    for (const auto block_y : algo::range(y_block_count))
    for (const auto block_x : algo::range(x_block_count))
    {
        const size_t block_x1 = block_x * block_size;
        const size_t block_y1 = block_y * block_size;
        const size_t block_x2 = std::min(block_x1 + block_size, header.width);
        const size_t block_y2 = std::min(block_y1 + block_size, header.height);
        if (!bit_mask)
        {
            control = control_block2_stream.read<u8>();
            bit_mask = 0x80;
        }
        if (!(control & bit_mask))
        {
            for (const auto y : algo::range(block_y1, block_y2))
            for (const auto x : algo::range(block_x1, block_x2))
            {
                output_image.at(x, y) = res::read_pixel
                    <res::PixelFormat::BGRA8888>(data_block2_stream);
            }
        }
        bit_mask >>= 1;
    }
    return output_image;
}

static std::unique_ptr<io::BaseByteStream> decrypt(const bstr &input)
{
    auto output_stream = std::make_unique<io::MemoryByteStream>(input);
    output_stream->seek(output_stream->size() - 0x2F);
    const auto tail_key = output_stream->read(0x2C);
    const auto pair_key = output_stream->read(2);
    auto encrypted_data = output_stream->seek(8).read(0x2C);
    for (const auto i : algo::range(0, 0x2C, 2))
    {
        for (const auto j : algo::range(2))
        {
            encrypted_data[i + j] ^= pair_key[j];
            encrypted_data[i + j] -= tail_key[i + j];
        }
    }
    output_stream->seek(8).write(encrypted_data);
    output_stream->seek(0);
    return std::move(output_stream);
}

static Header read_header(io::BaseByteStream &input_stream)
{
    input_stream.seek(0x18);
    Header header;
    header.sub_type = input_stream.read_le<u32>();
    header.main_type = input_stream.read_le<u16>();
    header.width = input_stream.read_le<u16>();
    header.height = input_stream.read_le<u16>();
    header.depth = input_stream.read_le<u16>();
    return header;
}

bool Pb3ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Pb3ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto decrypted_stream = decrypt(input_file.stream.seek(0).read_to_eof());
    const auto header = read_header(*decrypted_stream);

    if (header.main_type == 1 && header.sub_type == 0x10)
        return unpack_v1(header, *decrypted_stream);

    if (header.main_type == 2)
        return unpack_v2(header, *decrypted_stream);

    if (header.main_type == 3)
        return unpack_v3(header, *decrypted_stream);

    if (header.main_type == 5)
        return unpack_v5(header, *decrypted_stream);

    if (header.main_type == 6)
        return unpack_v6(logger, header, *decrypted_stream);

    throw err::NotSupportedError(algo::format(
        "Unsupported type: %d.%d", header.main_type, header.sub_type));
}

static auto _ = dec::register_decoder<Pb3ImageDecoder>("purple-software/pb3");
