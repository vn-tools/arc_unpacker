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

#include "algo/pack/lzss.h"
#include <array>
#include "algo/ptr.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;

namespace
{
    struct LzssEncoderState final
    {
        LzssEncoderState(const size_t dict_size, const size_t max_match_size);

        void insert_node(int r);
        void delete_node(int p);

        const size_t dict_size;
        const size_t max_match_size;
        const int empty;
        std::vector<u8> text_buf;
        std::vector<int> children[2], dad;

        size_t match_position;
        size_t match_size;
    };

    class BitwiseLzssWriter final : public algo::pack::BaseLzssWriter
    {
    public:
        BitwiseLzssWriter();
        void write_literal(const u8 literal) override;
        void write_repetition(
            const size_t position_bits,
            const size_t position,
            const size_t size_bits,
            const size_t size) override;
        bstr retrieve() override;
    private:
        io::MemoryByteStream byte_stream;
        io::MsbBitStream bit_stream;
    };

    class BytewiseLzssWriter final : public algo::pack::BaseLzssWriter
    {
    public:
        BytewiseLzssWriter();
        void write_literal(const u8 literal) override;
        void write_repetition(
            const size_t position_bits,
            const size_t position,
            const size_t size_bits,
            const size_t size) override;
        bstr retrieve() override;

    private:
        void flush();
        io::MemoryByteStream byte_stream;
        size_t count;
        u8 control;
        std::vector<u8> states;
    };
}

BitwiseLzssWriter::BitwiseLzssWriter() : bit_stream(byte_stream)
{
}

void BitwiseLzssWriter::write_literal(const u8 literal)
{
    bit_stream.write(1, 1);
    bit_stream.write(8, literal);
}

void BitwiseLzssWriter::write_repetition(
    const size_t position_bits,
    const size_t position,
    const size_t size_bits,
    const size_t size)
{
    bit_stream.write(1, 0);
    bit_stream.write(position_bits, position);
    bit_stream.write(size_bits, size);
}

bstr BitwiseLzssWriter::retrieve()
{
    bit_stream.flush();
    return byte_stream.seek(0).read_to_eof();
}

BytewiseLzssWriter::BytewiseLzssWriter() : count(0), control(0)
{
}

void BytewiseLzssWriter::write_literal(const u8 literal)
{
    control >>= 1;
    control |= 0x80;
    count++;
    states.push_back(literal);
    if (count >= 8)
        flush();
}

void BytewiseLzssWriter::write_repetition(
    const size_t position_bits,
    const size_t position,
    const size_t size_bits,
    const size_t size)
{
    control >>= 1;
    count++;
    states.push_back(position & 0xFF);
    states.push_back(((position >> 8) << 4) | size);
    if (count >= 8)
        flush();
}

void BytewiseLzssWriter::flush()
{
    if (count == 0)
        return;
    control >>= 8 - count;
    byte_stream.write<u8>(control);
    for (const auto &state : states)
        byte_stream.write<u8>(state);
    count = 0;
    control = 0;
    states.clear();
}

bstr BytewiseLzssWriter::retrieve()
{
    flush();
    return byte_stream.seek(0).read_to_eof();
}

algo::pack::BytewiseLzssSettings::BytewiseLzssSettings()
    : initial_dictionary_pos(0xFEE)
{
}

bstr algo::pack::lzss_decompress(
    const bstr &input,
    const size_t output_size,
    const BitwiseLzssSettings &settings)
{
    io::MsbBitStream bit_stream(input);
    return lzss_decompress(bit_stream, output_size, settings);
}

bstr algo::pack::lzss_decompress(
    io::BaseBitStream &input_stream,
    const size_t output_size,
    const BitwiseLzssSettings &settings)
{
    std::vector<u8> dict(1 << settings.position_bits, 0);
    auto dict_ptr
        = algo::make_cyclic_ptr(dict.data(), dict.size())
        + settings.initial_dictionary_pos;

    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);
    while (output_ptr.left())
    {
        if (input_stream.read(1))
        {
            const auto b = input_stream.read(8);
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        else
        {
            auto look_behind_pos = input_stream.read(settings.position_bits);
            auto repetitions = input_stream.read(settings.size_bits)
                + settings.min_match_size;
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
    }
    return output;
}

bstr algo::pack::lzss_decompress(
    const bstr &input,
    const size_t output_size,
    const BytewiseLzssSettings &settings)
{
    std::array<u8, 0x1000> dict = {0};
    auto dict_ptr
        = algo::make_cyclic_ptr(dict.data(), dict.size())
        + settings.initial_dictionary_pos;

    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);
    auto input_ptr = algo::make_ptr(input);

    u16 control = 0;
    while (output_ptr.left())
    {
        control >>= 1;
        if (!(control & 0x100))
        {
            if (!input_ptr.left()) break;
            control = *input_ptr++ | 0xFF00;
        }
        if (control & 1)
        {
            if (!input_ptr.left()) break;
            const auto b = *input_ptr++;
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        else
        {
            if (input_ptr.left() < 2) break;
            const auto lo = *input_ptr++;
            const auto hi = *input_ptr++;
            const auto look_behind_pos = lo | ((hi & 0xF0) << 4);
            auto repetitions = (hi & 0xF) + 3;
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
    }
    return output;
}

LzssEncoderState::LzssEncoderState(
    const size_t dict_size, const size_t max_match_size) :
        dict_size(dict_size),
        max_match_size(max_match_size),
        empty(dict_size),
        text_buf(dict_size + max_match_size + 1),
        dad(dict_size + 1)
{
    children[0].resize(dict_size + 1);
    children[1].resize(dict_size + 1 + 256);
    for (auto &c : children[0]) c = empty;
    for (auto &c : children[1]) c = empty;
    for (auto &c : dad) c = empty;
}

void LzssEncoderState::insert_node(int r)
{
    int cmp = 1;
    int p = dict_size + 1 + text_buf[r];
    children[0][r] = children[1][r] = empty;
    match_size = 0;
    while (true)
    {
        const auto lr = cmp >= 0;
        if (children[lr][p] == empty)
        {
            children[lr][p] = r;
            dad[r] = p;
            return;
        }
        p = children[lr][p];
        size_t i = 0;
        while (++i < max_match_size)
            if ((cmp = text_buf[r + i] - text_buf[p + i]) != 0)
                break;
        if (i > match_size)
        {
            match_position = p;
            if ((match_size = i) >= max_match_size)
                break;
        }
    }
    dad[r] = dad[p];
    children[0][r] = children[0][p];
    children[1][r] = children[1][p];
    dad[children[0][p]] = r;
    dad[children[1][p]] = r;
    children[children[1][dad[p]] == p][dad[p]] = r;
    dad[p] = empty;
}

void LzssEncoderState::delete_node(int p)
{
    if (dad[p] == empty)
        return;

    int q;
    if (children[1][p] == empty)
        q = children[0][p];
    else if (children[0][p] == empty)
        q = children[1][p];
    else
    {
        q = children[0][p];
        if (children[1][q] != empty)
        {
            do
                q = children[1][q];
            while (children[1][q] != empty);

            children[1][dad[q]] = children[0][q];
            dad[children[0][q]] = dad[q];
            children[0][q] = children[0][p];
            dad[children[0][p]] = q;
        }

        children[1][q] = children[1][p];
        dad[children[1][p]] = q;
    }

    dad[q] = dad[p];
    children[children[1][dad[p]] == p][dad[p]] = q;
    dad[p] = empty;
}

bstr algo::pack::lzss_compress(
    io::BaseByteStream &input_stream,
    const algo::pack::BitwiseLzssSettings &settings,
    algo::pack::BaseLzssWriter &writer)
{
    const auto dict_size = 1 << settings.position_bits;
    const auto max_match_size
        = settings.min_match_size + (1 << settings.size_bits) - 1;
    LzssEncoderState state(dict_size, max_match_size);

    size_t s = 0;
    size_t r = dict_size - max_match_size;
    for (const auto i : algo::range(s, r))
        state.text_buf[i] = 0;

    size_t len;
    for (len = 0; len < max_match_size && input_stream.left(); len++)
        state.text_buf[r + len] = input_stream.read<u8>();

    for (const auto i : algo::range(1, max_match_size + 1))
        state.insert_node(r - i);
    state.insert_node(r);

    while (len > 0)
    {
        auto match_size = state.match_size;
        auto match_position = state.match_position;
        if (match_size > len)
            match_size = len;
        if (match_size < settings.min_match_size)
        {
            match_size = 1;
            writer.write_literal(state.text_buf[r]);
        }
        else
        {
            match_position -= dict_size - max_match_size;
            match_position += settings.initial_dictionary_pos;
            match_position %= dict_size;
            writer.write_repetition(
                settings.position_bits,
                match_position,
                settings.size_bits,
                match_size - settings.min_match_size);
        }

        auto last_match_size = match_size;
        size_t i;
        for (i = 0; i < last_match_size && input_stream.left(); i++)
        {
            const auto c = input_stream.read<u8>();
            state.delete_node(s);
            state.text_buf[s] = c;
            if (s < max_match_size + 1)
                state.text_buf[s + dict_size] = c;
            s = (s + 1) % dict_size;
            r = (r + 1) % dict_size;
            state.insert_node(r);
        }
        while (i++ < last_match_size)
        {
            state.delete_node(s);
            s = (s + 1) % dict_size;
            r = (r + 1) % dict_size;
            if (--len)
                state.insert_node(r);
        }
    }
    return writer.retrieve();
}

bstr algo::pack::lzss_compress(
    const bstr &input, const algo::pack::BitwiseLzssSettings &settings)
{
    io::MemoryByteStream input_stream(input);
    return algo::pack::lzss_compress(input_stream, settings);
}

bstr algo::pack::lzss_compress(
    io::BaseByteStream &input_stream,
    const algo::pack::BitwiseLzssSettings &settings)
{
    BitwiseLzssWriter writer;
    return algo::pack::lzss_compress(input_stream, settings, writer);
}

bstr algo::pack::lzss_compress(
    const bstr &input, const algo::pack::BytewiseLzssSettings &settings)
{
    io::MemoryByteStream input_stream(input);
    return algo::pack::lzss_compress(input_stream, settings);
}

bstr algo::pack::lzss_compress(
    io::BaseByteStream &input_stream,
    const algo::pack::BytewiseLzssSettings &settings)
{
    BitwiseLzssSettings bitwise_settings;
    bitwise_settings.min_match_size = 3;
    bitwise_settings.position_bits = 12;
    bitwise_settings.size_bits = 4;
    bitwise_settings.initial_dictionary_pos = settings.initial_dictionary_pos;
    BytewiseLzssWriter writer;
    return algo::pack::lzss_compress(input_stream, bitwise_settings, writer);
}
