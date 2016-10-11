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

#include "dec/cri/hca_audio_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/cri/hca/ath_table.h"
#include "dec/cri/hca/channel_decoder.h"
#include "dec/cri/hca/meta.h"
#include "dec/cri/hca/permutator.h"
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::cri;
using namespace au::dec::cri::hca;

static const bstr magic = "HCA\x00"_b;

static inline f32 clamp(const f32 input)
{
    if (input > 1)
        return 1;
    if (input < -1)
        return -1;
    return input;
}

static inline unsigned int ceil2(unsigned int a, unsigned int b)
{
    if (b <= 0)
        return 0;
    return a / b + ((a % b) ? 1 : 0);
}

static u16 crc16(const bstr &data)
{
    static u16 table[] =
    {
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
        0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
        0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
        0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
        0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
        0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
        0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
        0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
        0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
        0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
        0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
        0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
        0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
        0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
        0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
        0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
        0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
        0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
        0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
        0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
        0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
        0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
        0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
        0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202,
    };

    u16 checksum = 0;
    for (const auto c : data)
        checksum = (checksum << 8) ^ table[(checksum >> 8) ^ c];

    return checksum;
}

static std::vector<u8> get_types(
    const Meta &meta, const std::array<u8, 9> &params)
{
    std::vector<u8> types(0x10);
    const auto span = meta.fmt->channel_count / std::max<u8>(1, params[2]);
    if (!params[6] || span <= 1)
        return types;

    for (const auto i : algo::range(params[2]))
    {
        const auto idx = i * span;

        if (span >= 2)
        {
            types.at(idx + 0) = 1;
            types.at(idx + 1) = 2;
        }

        if (span == 4 && params[3] == 0)
        {
            types.at(idx + 2) = 1;
            types.at(idx + 3) = 2;
        }

        if (span == 5 && params[3] <= 2)
        {
            types.at(idx + 3) = 1;
            types.at(idx + 4) = 2;
        }

        if (span == 6 || span == 7 || span == 8)
        {
            types.at(idx + 4) = 1;
            types.at(idx + 5) = 2;
        }

        if (span == 8)
        {
            types.at(idx + 6) = 1;
            types.at(idx + 7) = 2;
        }
    }

    return types;
}

static void decode_block(
    const Meta &meta,
    const AthTable &ath_table,
    std::vector<std::shared_ptr<ChannelDecoder>> &channel_decoders,
    const std::array<u8, 9> params,
    const bstr &block_data)
{
    if (crc16(block_data) != 0)
        throw err::CorruptDataError("Block checksum failed");

    // suspicion: I believe the last 2 bytes are used as a CRC16 manipulator
    // (so that the checksum computes to 0.)
    io::MsbBitStream bit_stream(block_data.substr(0, block_data.size()));

    int magic = bit_stream.read(16);
    if (magic == 0xFFFF)
    {
        int tmp = (bit_stream.read(9) << 8) - bit_stream.read(7);
        for (const auto i : algo::range(meta.fmt->channel_count))
        {
            channel_decoders[i]->decode1(
                bit_stream, params[8], tmp, ath_table);
        }

        for (const auto i : algo::range(8))
        {
            for (const auto j : algo::range(meta.fmt->channel_count))
                channel_decoders[j]->decode2(bit_stream);

            for (const auto j : algo::range(meta.fmt->channel_count))
            {
                channel_decoders[j]->decode3(
                    params[8],
                    params[7],
                    params[6] + params[5],
                    params[4]);
            }

            for (const auto j : algo::range(meta.fmt->channel_count - 1))
            {
                channel_decoders[j]->decode4(
                    i,
                    params[4] - params[5],
                    params[5],
                    params[6],
                    *channel_decoders[j + 1]);
            }

            for (const auto j : algo::range(meta.fmt->channel_count))
                channel_decoders[j]->decode5(i);
        }
    }
}

bool HcaAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Audio HcaAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    // TODO when testable: this should be customizable.
    const u32 ciph_key1 = 0x30DBE1AB;
    const u32 ciph_key2 = 0xCC554639;

    input_file.stream.seek(6);
    static const u16 meta_size = input_file.stream.read_be<u16>();

    input_file.stream.seek(0);
    auto meta = read_meta(input_file.stream.read(meta_size));

    if (!meta.hca) throw err::CorruptDataError("Missing 'hca' chunk");
    if (!meta.fmt) throw err::CorruptDataError("Missing 'fmt' chunk");
    if (!meta.rva) throw err::CorruptDataError("Missing 'rva' chunk");
    if (!meta.ath) throw err::CorruptDataError("Missing 'ath' chunk");
    if (!meta.ciph) throw err::CorruptDataError("Missing 'ciph' chunk");
    if (!meta.comp) throw err::CorruptDataError("Missing 'comp' chunk");

    if (meta.fmt->channel_count < 1 || meta.fmt->channel_count >= 16)
        throw err::UnsupportedChannelCountError(meta.fmt->channel_count);

    const auto sample_rate = meta.fmt->sample_rate;
    const auto channel_count = meta.fmt->channel_count;
    const auto block_size = meta.comp->block_size;
    const auto block_count = meta.fmt->block_count;
    const auto volume = meta.rva->volume;

    AthTable ath_table(meta.ath->type, sample_rate);
    Permutator permutator(meta.ciph->type, ciph_key1, ciph_key2);

    std::array<u8, 9> params;
    for (const auto i : algo::range(8))
        params[i] = meta.comp->unk[i];
    if (params[0] != 1 || params[1] != 15)
        throw err::CorruptDataError("Unsupported decoder params");
    params[8] = ceil2(params[4] - (params[5] + params[6]), params[7]);

    const auto types = get_types(meta, params);
    std::vector<std::shared_ptr<ChannelDecoder>> channel_decoders;
    for (const auto i : algo::range(channel_count))
    {
        auto channel_decoder = std::make_shared<ChannelDecoder>(
            types[i],
            params[5] + params[6],
            params[5] + ((types[i] != 2) ? params[6] : 0));
        channel_decoders.push_back(channel_decoder);
    }

    input_file.stream.seek(meta.hca->data_offset);
    std::vector<s16> samples;
    samples.reserve(128 * 8 * channel_count * block_count);
    for (const auto b : algo::range(block_count))
    {
        decode_block(
            meta,
            ath_table,
            channel_decoders,
            params,
            permutator.permute(input_file.stream.read(block_size)));

        for (const auto i : algo::range(8))
        for (const auto j : algo::range(128))
        for (const auto k : algo::range(channel_count))
        {
            const auto value = clamp(channel_decoders[k]->wave[i][j]);
            samples.push_back(static_cast<s16>(value * 0x7FFF));
        }
    }

    res::Audio audio;
    audio.codec = 1;
    audio.channel_count = channel_count;
    audio.sample_rate = sample_rate;
    audio.bits_per_sample = 16;
    audio.samples
        = bstr(reinterpret_cast<const u8*>(samples.data()), samples.size() * 2);
    if (meta.loop)
    {
        audio.loops.push_back(res::AudioLoopInfo
        {
            meta.loop->start * 8 * 128 * sample_rate,
            meta.loop->end * 8 * 128 * sample_rate,
            meta.loop->repetitions == 128 ? 0 : meta.loop->repetitions,
        });
    }
    return audio;
}

static auto _ = dec::register_decoder<HcaAudioDecoder>("cri/hca");
