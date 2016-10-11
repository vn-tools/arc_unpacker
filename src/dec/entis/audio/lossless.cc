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

#include "dec/entis/audio/lossless.h"
#include "algo/range.h"
#include "dec/entis/common/huffman_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis;
using namespace au::dec::entis::audio;

static bstr decode_chunk_pcm8(
    const MioHeader &header,
    const MioChunk &chunk,
    common::BaseDecoder &decoder)
{
    throw err::NotSupportedError("PCM8 is not supported");
}

static bstr decode_chunk_pcm16(
    const MioHeader &header,
    const MioChunk &chunk,
    common::BaseDecoder &decoder)
{
    const auto sample_count = chunk.sample_count;
    const auto channel_count = header.channel_count;
    const auto total_sample_count = sample_count * channel_count;

    if (chunk.initial)
        decoder.reset();

    bstr decoded(total_sample_count * sizeof(s16));
    decoder.set_input(chunk.data);
    decoder.decode(decoded.get<u8>(), decoded.size());

    bstr mixed(total_sample_count * sizeof(s16));
    for (const auto i : algo::range(channel_count))
    {
        const auto offset = i * sample_count * sizeof(s16);
        u8 *source1_ptr = decoded.get<u8>() + offset;
        u8 *source2_ptr = source1_ptr + sample_count;
        u8 *target_ptr = mixed.get<u8>() + offset;
        for (const auto j : algo::range(sample_count))
        {
            s8 bytLow = source2_ptr[j];
            s8 bytHigh = source1_ptr[j];
            target_ptr[j * sizeof(s16) + 0] = bytLow;
            target_ptr[j * sizeof(s16) + 1] = bytHigh ^ (bytLow >> 7);
        }
    }

    bstr output(total_sample_count * sizeof(s16));
    const auto *source_ptr = mixed.get<s16>();
    const auto step = channel_count;
    for (const auto i : algo::range(channel_count))
    {
        auto target_ptr = output.get<s16>() + i;
        s16 value = 0;
        s16 delta = 0;
        for (const auto j : algo::range(sample_count))
        {
            delta += *source_ptr++;
            value += delta;
            *target_ptr = value;
            target_ptr += step;
        }
    }

    return output;
}

struct LosslessAudioDecoder::Priv final
{
    Priv(const MioHeader &header);

    const MioHeader &header;
    common::HuffmanDecoder decoder;
};

LosslessAudioDecoder::Priv::Priv(const MioHeader &header) : header(header)
{
}

LosslessAudioDecoder::LosslessAudioDecoder(const MioHeader &header)
    : p(new Priv(header))
{
    if (header.architecture != common::Architecture::RunLengthHuffman)
        throw err::CorruptDataError("Expected Huffman decoder");
}

LosslessAudioDecoder::~LosslessAudioDecoder()
{
}

bstr LosslessAudioDecoder::process_chunk(const MioChunk &chunk)
{
    if (p->header.bits_per_sample == 16)
        return decode_chunk_pcm16(p->header, chunk, p->decoder);

    if (p->header.bits_per_sample == 8)
        return decode_chunk_pcm8(p->header, chunk, p->decoder);

    throw err::UnsupportedBitDepthError(p->header.bits_per_sample);
}
