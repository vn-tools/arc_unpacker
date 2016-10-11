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

#include "dec/kaguya/bmr_file_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::kaguya;

static const bstr magic = "BMR"_b;

namespace
{
    struct Tree final
    {
        size_t nodes[2][256];
        size_t root;
    };
}

static size_t fill_huffman_tree(
    Tree &tree, io::BaseBitStream &input_stream)
{
    if (!input_stream.read(1))
        return input_stream.read(8);
    const auto b = tree.root++;
    tree.nodes[0][b - 256] = fill_huffman_tree(tree, input_stream);
    tree.nodes[1][b - 256] = fill_huffman_tree(tree, input_stream);
    return b;
}

static Tree create_huffman_tree(io::BaseBitStream &input_stream)
{
    Tree t;
    t.root = 256;
    t.root = fill_huffman_tree(t, input_stream);
    return t;
}

static bstr decompress_huffman(
    io::BaseBitStream &input_stream, const size_t output_size)
{
    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);
    auto tree = create_huffman_tree(input_stream);
    while (output_ptr.left())
    {
        size_t symbol = tree.root;
        while (symbol >= 0x100)
            symbol = tree.nodes[input_stream.read(1)][symbol - 256];
        *output_ptr++ = symbol;
    }
    return output;
}

static bstr decompress_rle(
    const bstr &input, const size_t step, const size_t output_size)
{
    bstr output(output_size);
    auto input_ptr = algo::make_ptr(input);
    for (const auto i : algo::range(step))
    {
        auto output_ptr = algo::make_ptr(output) + i;
        if (!input_ptr.left() || !output_ptr.left())
            return output;

        u8 b1 = *input_ptr++;
        *output_ptr = b1;
        output_ptr += step;

        while (output_ptr.left() && input_ptr.left())
        {
            u8 b2 = *input_ptr++;
            *output_ptr = b2;
            output_ptr += step;

            if (b2 == b1)
            {
                if (!input_ptr.left())
                    return output;
                size_t count = *input_ptr++;
                if (count & 0x80)
                {
                    if (!input_ptr.left())
                        return output;
                    count = *input_ptr++ + ((count & 0x7F) << 8) + 128;
                }

                while (count-- && output_ptr.left())
                {
                    *output_ptr = b2;
                    output_ptr += step;
                }

                if (input_ptr.left() && output_ptr.left())
                {
                    b2 = *input_ptr++;
                    *output_ptr = b2;
                    output_ptr += step;
                }
            }
            b1 = b2;
        }
    }
    return output;
}

static bstr decrypt(const bstr &input, const u32 key)
{
    int freq_table[256] = {0};
    for (const auto c : input)
        freq_table[c]++;
    for (const auto i : algo::range(1, 256))
        freq_table[i] += freq_table[i - 1];
    std::vector<int> distribution_table(input.size());
    for (const auto i : algo::range(input.size() - 1, -1, -1))
    {
        const auto b = input[i];
        freq_table[b]--;
        distribution_table.at(freq_table[b]) = i;
    }
    size_t pos = key;
    bstr output(input.size());
    for (const auto i : algo::range(input.size()))
    {
        pos = distribution_table.at(pos);
        output[i] = input.at(pos);
    }
    return output;
}

static bstr descramble(const bstr &input)
{
    u8 scramble[256];
    for (const auto i : algo::range(256))
        scramble[i] = i;
    bstr output(input);
    for (const auto i : algo::range(output.size()))
    {
        const auto b = output[i];
        output[i] = scramble[b];
        for (int j = b; j > 0; --j)
            scramble[j] = scramble[j - 1];
        scramble[0] = output[i];
    }
    return output;
}

bool BmrFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> BmrFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto step = input_file.stream.read<u8>();
    const auto size_after_rle = input_file.stream.read_le<u32>();
    const auto key = input_file.stream.read_le<u32>();
    const auto size_after_huffman = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    io::MsbBitStream bit_stream(input_file.stream);

    auto data = decompress_huffman(bit_stream, size_after_huffman);
    data = descramble(data);
    data = decrypt(data, key);
    if (step)
        data = decompress_rle(data, step, size_after_rle);

    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<BmrFileDecoder>("kaguya/bmr");
