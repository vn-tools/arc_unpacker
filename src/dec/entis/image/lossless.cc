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

#include "dec/entis/image/lossless.h"
#include <functional>
#include "algo/range.h"
#include "dec/entis/common/erisa_decoder.h"
#include "dec/entis/common/gamma_decoder.h"
#include "dec/entis/common/huffman_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis;
using namespace au::dec::entis::image;

namespace
{
    struct DecodeContext final
    {
        u8 eri_version;
        u8 op_table;
        u8 encode_type;
        u8 bit_count;

        size_t block_size;
        size_t block_area;
        size_t block_samples;
        size_t channel_count;
        size_t block_stride;

        size_t width_blocks;
        size_t height_blocks;
    };

    using Permutation = std::vector<int>;

    using ColorTransformer = std::function<void(u8 *, const DecodeContext &)>;
}

static Permutation init_permutation(const DecodeContext &ctx)
{
    Permutation permutation(ctx.block_samples * 4);

    auto permutation_ptr = permutation.data();
    for (const auto c : algo::range(ctx.channel_count))
    for (const auto y : algo::range(ctx.block_size))
    for (const auto x : algo::range(ctx.block_size))
        *permutation_ptr++ = c * ctx.block_area + y * ctx.block_size + x;

    for (const auto c : algo::range(ctx.channel_count))
    for (const auto y : algo::range(ctx.block_size))
    for (const auto x : algo::range(ctx.block_size))
        *permutation_ptr++ = c * ctx.block_area + y + x * ctx.block_size;

    for (const auto y : algo::range(ctx.block_size))
    for (const auto x : algo::range(ctx.block_size))
    for (const auto c : algo::range(ctx.channel_count))
        *permutation_ptr++ = c * ctx.block_area + y * ctx.block_size + x;

    for (const auto y : algo::range(ctx.block_size))
    for (const auto x : algo::range(ctx.block_size))
    for (const auto c : algo::range(ctx.channel_count))
        *permutation_ptr++ = c * ctx.block_area + y + x * ctx.block_size;

    return permutation;
}

static size_t get_channel_count(const EriHeader &header)
{
    switch (header.format_type)
    {
        case EriFormatType::Colored:
            if (header.bit_depth <= 8)
                return 1;
            if (header.format_flags & EriFormatFlags::WithAlpha)
                return 4;
            return 3;

        case EriFormatType::Gray:
            return 1;
    }
    throw err::CorruptDataError("Unknown pixel format");
}

static void color_op_0000(u8 *decode_buf, const DecodeContext &context)
{
}

static void color_op_0101(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
        decode_buf[i + context.block_area] += decode_buf[i];
}

static void color_op_0110(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
        decode_buf[i + context.block_area * 2] += decode_buf[i];
}

static void color_op_0111(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
    {
        decode_buf[i + context.block_area] += decode_buf[i];
        decode_buf[i + context.block_area * 2] += decode_buf[i];
    }
}

static void color_op_1001(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
        decode_buf[i] += decode_buf[i + context.block_area];
}

static void color_op_1010(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
    {
        decode_buf[i + context.block_area * 2]
            += decode_buf[i + context.block_area];
    }
}

static void color_op_1011(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
    {
        decode_buf[i] += decode_buf[i + context.block_area];
        decode_buf[i + context.block_area * 2]
            += decode_buf[i + context.block_area];
    }
}

static void color_op_1101(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
        decode_buf[i] += decode_buf[i + context.block_area * 2];
}

static void color_op_1110(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
    {
        decode_buf[i + context.block_area]
            += decode_buf[i + context.block_area * 2];
    }
}

static void color_op_1111(u8 *decode_buf, const DecodeContext &context)
{
    for (const auto i : algo::range(context.block_area))
    {
        decode_buf[i] += decode_buf[i + context.block_area * 2];
        decode_buf[i + context.block_area]
            += decode_buf[i + context.block_area * 2];
    }
}

static std::vector<ColorTransformer> color_ops
{
    color_op_0000, color_op_0000, color_op_0000, color_op_0000,
    color_op_0000, color_op_0101, color_op_0110, color_op_0111,
    color_op_0000, color_op_1001, color_op_1010, color_op_1011,
    color_op_0000, color_op_1101, color_op_1110, color_op_1111,
};

static void transform(
    const u8 transformer_code,
    const DecodeContext &ctx,
    const Permutation &permutation,
    const bstr &arrange_buf,
    u8 *prev_block_row,
    u8 *prev_block_col,
    bstr &block_out)
{
    const auto diff_mode   = (transformer_code & 0b11'00'0000) >> 6;
    const auto perm_offset = (transformer_code & 0b00'11'0000) >> 4;
    const auto color_op    = (transformer_code & 0b00'00'1111);

    for (const auto i : algo::range(ctx.block_samples))
    {
        block_out[permutation[perm_offset * ctx.block_samples + i]]
            = arrange_buf[i];
    }
    if (!transformer_code)
        return;

    color_ops[color_op](block_out.get<u8>(), ctx);

    if (diff_mode & 0b01)
    {
        auto block_out_ptr = block_out.get<u8>();
        auto next_block_col_ptr = prev_block_col;
        for (const auto i : algo::range(ctx.block_stride))
        {
            u8 last_value = *next_block_col_ptr;
            for (const auto j : algo::range(ctx.block_size))
            {
                last_value += *block_out_ptr;
                *block_out_ptr++ = last_value;
            }
            *next_block_col_ptr++ = last_value;
        }
    }
    else
    {
        auto block_out_ptr = block_out.get<u8>();
        auto next_block_col_ptr = prev_block_col;
        for (const auto i : algo::range(ctx.block_stride))
        {
            *next_block_col_ptr++ = block_out_ptr[ctx.block_size - 1];
            block_out_ptr += ctx.block_size;
        }
    }

    auto prev_block_row_base = prev_block_row;
    auto block_out_ptr = block_out.get<u8>();
    for (const auto k : algo::range(ctx.channel_count))
    {
        const auto *prev_block_row_ptr = prev_block_row_base;
        for (const auto i : algo::range(ctx.block_size))
        {
            auto last_block_out_ptr = block_out_ptr;
            for (const auto j : algo::range(ctx.block_size))
                *block_out_ptr++ += *prev_block_row_ptr++;
            prev_block_row_ptr = last_block_out_ptr;
        }
        for (const auto j : algo::range(ctx.block_size))
            *prev_block_row_base++ = *prev_block_row_ptr++;
    }
}

static u8 get_transformer_code(
    const EriHeader &header,
    const DecodeContext &ctx,
    common::BaseDecoder &decoder,

    const u8 *&transformer_codes_ptr,
    common::HuffmanTree &huffman_tree,
    common::ProbModel &prob_model)
{
    if (ctx.channel_count < 3)
    {
        if (!(ctx.encode_type & 0x01) && header.architecture
            == common::Architecture::RunLengthGamma)
        {
            decoder.reset();
        }
        if (header.format_type == EriFormatType::Gray)
            return 0b11'00'0000;
        return 0b00'00'0000;
    }

    if (ctx.encode_type & 1)
        return *transformer_codes_ptr++;

    if (header.architecture == common::Architecture::RunLengthHuffman)
        return get_huffman_code(*decoder.bit_stream, huffman_tree);

    if (header.architecture == common::Architecture::Nemesis)
    {
        return static_cast<common::BaseErisaDecoder&>(decoder)
            .decode_erisa_code(prob_model);
    }

    if (header.architecture == common::Architecture::RunLengthGamma)
    {
        const auto transformer_code
            = 0b11'00'0000 | decoder.bit_stream->read(4);
        decoder.reset();
        return transformer_code;
    }

    throw err::NotSupportedError("Architecture not supported");
}

static std::vector<u8> prefetch_transformer_codes(
    const DecodeContext &ctx,
    const EriHeader &header,
    common::BaseDecoder &decoder,
    common::HuffmanTree &huffman_tree)
{
    std::vector<u8> transformer_codes;
    if (!(ctx.encode_type & 0x01) || (ctx.channel_count < 3))
        return transformer_codes;
    for (const auto i : algo::range(ctx.width_blocks * ctx.height_blocks))
    {
        u8 op_code;
        if (header.architecture == common::Architecture::RunLengthGamma)
        {
            op_code = 0b11'00'0000 | decoder.bit_stream->read(4);
        }
        else if (header.architecture == common::Architecture::RunLengthHuffman)
        {
            op_code = get_huffman_code(*decoder.bit_stream, huffman_tree);
        }
        else
        {
            throw err::NotSupportedError("Architecture not supported");
        }
        transformer_codes.push_back(op_code);
    }
    return transformer_codes;
}

static void validate_ctx(const DecodeContext &ctx, const EriHeader &header)
{
    if (ctx.op_table || (ctx.encode_type & 0xFE))
        throw err::CorruptDataError("Unexpected meta data");

    switch (ctx.eri_version)
    {
        case 1:
            if (ctx.bit_count != 0)
                throw err::UnsupportedBitDepthError(ctx.bit_count);
            break;
        case 8:
            if (ctx.bit_count != 8)
                throw err::UnsupportedBitDepthError(ctx.bit_count);
            break;
        case 16:
            if (ctx.bit_count != 8 || ctx.encode_type)
                throw err::UnsupportedBitDepthError(ctx.bit_count);
            break;
        default:
            throw err::UnsupportedVersionError(ctx.eri_version);
    }

    if (!header.blocking_degree)
        throw err::CorruptDataError("Blocking degree not set");
}

static bstr crop(
    const bstr &input,
    const DecodeContext &ctx,
    const EriHeader &header)
{
    bstr output(header.width * header.height * ctx.channel_count);
    for (const auto y : algo::range(header.height))
    {
        auto *output_ptr = output.get<u8>();
        const auto *input_ptr = input.get<u8>();
        output_ptr += y * header.width * ctx.channel_count;
        input_ptr += y * ctx.width_blocks * ctx.block_stride;
        for (const auto x : algo::range(header.width * ctx.channel_count))
            *output_ptr++ = *input_ptr++;
    }
    return output;
}

bstr image::decode_lossless_pixel_data(
    const EriHeader &header, common::BaseDecoder &decoder)
{
    DecodeContext ctx;
    ctx.eri_version = decoder.bit_stream->read(8);
    ctx.op_table = decoder.bit_stream->read(8);
    ctx.encode_type = decoder.bit_stream->read(8);
    ctx.bit_count = decoder.bit_stream->read(8);

    ctx.channel_count = get_channel_count(header);
    ctx.block_size = (1 << header.blocking_degree);
    ctx.block_area = ctx.block_size * ctx.block_size;
    ctx.block_samples = ctx.block_area * ctx.channel_count;
    ctx.block_stride = ctx.block_size * ctx.channel_count;

    ctx.width_blocks = (header.width + ctx.block_size - 1) / ctx.block_size;
    ctx.height_blocks = (header.height + ctx.block_size - 1) / ctx.block_size;

    validate_ctx(ctx, header);

    common::ProbModel prob_model; // for nemesis decoder
    common::HuffmanTree huffman_tree; // for huffman decoder

    const auto permutation = init_permutation(ctx);
    const auto transformer_codes = prefetch_transformer_codes(
        ctx, header, decoder, huffman_tree);

    if (decoder.bit_stream->read(1))
        throw err::CorruptDataError("Expected 0 bit");

    if (header.architecture == common::Architecture::RunLengthGamma)
    {
        if (ctx.encode_type & 0x01)
            decoder.reset();
    }
    else if (header.architecture == common::Architecture::RunLengthHuffman)
        decoder.reset();
    else if (header.architecture == common::Architecture::Nemesis)
        decoder.reset();
    else
        throw err::NotSupportedError("Architecture not supported");

    bstr output(ctx.width_blocks * ctx.height_blocks * ctx.block_samples);
    bstr arrange_buf(ctx.block_samples);
    bstr block_out(ctx.block_samples);
    bstr prev_col(ctx.height_blocks * ctx.block_stride);
    bstr prev_row(ctx.width_blocks * ctx.block_stride);

    auto transformer_codes_ptr = transformer_codes.data();
    for (const auto y : algo::range(ctx.height_blocks))
    for (const auto x : algo::range(ctx.width_blocks))
    {
        const u8 transformer_code = get_transformer_code(
            header,
            ctx,
            decoder,
            transformer_codes_ptr,
            huffman_tree,
            prob_model);

        decoder.decode(arrange_buf.get<u8>(), arrange_buf.size());

        transform(
            transformer_code,
            ctx,
            permutation,
            arrange_buf,
            prev_row.get<u8>() + x * ctx.block_stride,
            prev_col.get<u8>() + y * ctx.block_stride,
            block_out);

        auto *block_out_ptr = block_out.get<u8>();
        for (const auto c : algo::range(ctx.channel_count))
        for (const auto yy : algo::range(ctx.block_size))
        for (const auto xx : algo::range(ctx.block_size))
        {
            const auto sum_y = y * ctx.block_size + yy;
            const auto sum_x = x * ctx.block_size + xx;
            const auto pos = sum_y * ctx.width_blocks * ctx.block_size + sum_x;
            output[pos * ctx.channel_count + c] = *block_out_ptr++;
        }
    }

    return crop(output, ctx, header);
}
