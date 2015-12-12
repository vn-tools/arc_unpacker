#include "fmt/purple_software/pb3_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"
#include "ptr.h"
#include "util/cyclic_buffer.h"

using namespace au;
using namespace au::fmt::purple_software;

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
    bstr output(output_size);
    auto output_ptr = make_ptr(output);
    util::CyclicBuffer<0x800> dict(0x7DE);
    io::MemoryStream control_block_stream(control_block);
    io::MemoryStream data_block_stream(data_block);
    int control = 0, bit_mask = 0;
    while (output_ptr < output_ptr.end())
    {
        if (!bit_mask)
        {
            bit_mask = 0x80;
            control = control_block_stream.read_u8();
        }
        if (control & bit_mask)
        {
            const auto tmp = data_block_stream.read_u16_le();
            auto repetitions = (tmp & 0x1F) + 3;
            auto look_behind_pos = tmp >> 5;
            while (repetitions-- && output_ptr < output_ptr.end())
            {
                *output_ptr++ = dict[look_behind_pos++];
                dict << output_ptr[-1];
            }
        }
        else
        {
            *output_ptr++ = data_block_stream.read_u8();
            dict << output_ptr[-1];
        }
        bit_mask >>= 1;
    }
    return output;
}

static res::Image unpack_v1(const Header &header, io::Stream &input_stream)
{
    const auto channel_count = header.depth >> 3;
    const auto stride = header.width * channel_count;
    res::Image output_image(header.width, header.height);

    const auto main_sizes_offset = input_stream.seek(0x2C).read_u32_le();
    const auto data_sizes_offset = input_stream.seek(0x30).read_u32_le();

    input_stream.seek(main_sizes_offset);
    std::vector<size_t> main_sizes;
    for (const auto channel : algo::range(channel_count))
        main_sizes.push_back(input_stream.read_u32_le());

    input_stream.seek(data_sizes_offset);
    std::vector<size_t> data_sizes;
    for (const auto channel : algo::range(channel_count))
        data_sizes.push_back(input_stream.read_u32_le());

    std::vector<size_t> main_offsets;
    std::vector<size_t> data_offsets;
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
        const auto control_block1_size = input_stream.read_u32_le();
        const auto data_block1_size = input_stream.read_u32_le();
        const auto size_orig = input_stream.read_u32_le();

        const auto control_block1 = input_stream.read(control_block1_size);
        const auto data_block1 = input_stream.read(data_block1_size);
        const auto control_block2 = input_stream.read(
            main_offsets[channel] + main_sizes[channel] - input_stream.tell());

        const auto data_block2 = input_stream
            .seek(data_offsets[channel])
            .read(data_sizes[channel]);

        const auto plane = custom_lzss_decompress(
            control_block2, data_block2, size_orig);

        size_t x_block_count = header.width >> 4;
        size_t y_block_count = header.height >> 4;
        if (header.width & 0xF) x_block_count++;
        if (header.height & 0xF) y_block_count++;

        if (!y_block_count || !x_block_count)
            continue;

        io::MemoryStream control_block1_stream(control_block1);
        io::MemoryStream data_block1_stream(data_block1);
        io::MemoryStream plane_stream(plane);
        int bit_mask = 0, control = 0;
        for (const auto block_y : algo::range(y_block_count))
        for (const auto block_x : algo::range(x_block_count))
        {
            const size_t block_x1 = block_x * 16;
            const size_t block_y1 = block_y * 16;
            const size_t block_x2 = std::min(block_x1 + 16, header.width);
            const size_t block_y2 = std::min(block_y1 + 16, header.height);

            if (!bit_mask)
            {
                control = control_block1_stream.read_u8();
                bit_mask = 0x80;
            }
            if (control & bit_mask)
            {
                const auto b = data_block1_stream.read_u8();
                for (const auto y : algo::range(block_y1, block_y2))
                for (const auto x : algo::range(block_x1, block_x2))
                    output_image.at(x, y)[channel] = b;
            }
            else
            {
                for (const auto y : algo::range(block_y1, block_y2))
                for (const auto x : algo::range(block_x1, block_x2))
                    output_image.at(x, y)[channel] = plane_stream.read_u8();
            }
            bit_mask >>= 1;
        }
    }

    if (header.depth != 32)
        for (auto &c : output_image)
            c.a = 0xFF;
    return output_image;
}

static res::Image unpack_v5(const Header &header, io::Stream &input_stream)
{
    const auto channel_count = header.depth >> 3;
    const auto stride = header.width * channel_count;
    res::Image output_image(header.width, header.height);

    input_stream.seek(0x34);
    std::vector<size_t> control_offsets;
    std::vector<size_t> data_offsets;
    for (const auto i : algo::range(channel_count))
    {
        control_offsets.push_back(0x54 + input_stream.read_u32_le());
        data_offsets.push_back(0x54 + input_stream.read_u32_le());
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
        auto plane_ptr = make_ptr(plane);

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

static std::unique_ptr<io::Stream> decrypt(const bstr &input)
{
    auto output_stream = std::make_unique<io::MemoryStream>(input);
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

static Header read_header(io::Stream &input_stream)
{
    input_stream.seek(0x18);
    Header header;
    header.sub_type = input_stream.read_u32_le();
    header.main_type = input_stream.read_u16_le();
    header.width = input_stream.read_u16_le();
    header.height = input_stream.read_u16_le();
    header.depth = input_stream.read_u16_le();
    return header;
}

bool Pb3ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Pb3ImageDecoder::decode_impl(io::File &input_file) const
{
    auto decrypted_stream = decrypt(input_file.stream.seek(0).read_to_eof());
    const auto header = read_header(*decrypted_stream);

    if (header.main_type == 1 && header.sub_type == 0x10)
        return unpack_v1(header, *decrypted_stream);

    if (header.main_type == 5)
        return unpack_v5(header, *decrypted_stream);

    throw err::NotSupportedError(algo::format(
        "Unsupported type: %d.%d", header.main_type, header.sub_type));
}

static auto dummy = fmt::register_fmt<Pb3ImageDecoder>("purple-software/pb3");
