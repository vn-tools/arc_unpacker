#include "fmt/cri/hca_audio_decoder.h"
#include "err.h"
#include "fmt/cri/hca/ath_table.h"
#include "fmt/cri/hca/channel_decoder.h"
#include "fmt/cri/hca/custom_bit_reader.h"
#include "fmt/cri/hca/meta.h"
#include "fmt/cri/hca/permutator.h"
#include "io/bit_reader.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cri;
using namespace au::fmt::cri::hca;

static const bstr magic = "HCA\x00"_b;

static inline float clamp(const float input)
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

    throw err::NotSupportedError("Advanced type decoding is not supported");

    int idx = 0;
    for (const auto i : util::range(params[2]))
    {
        types.at(idx + 0) = 1;
        types.at(idx + 1) = 2;

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

        idx += span;
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
    CustomBitReader bit_reader(block_data.substr(0, block_data.size() - 2));

    int magic = bit_reader.get(16);
    if (magic == 0xFFFF)
    {
        int tmp = (bit_reader.get(9) << 8) - bit_reader.get(7);
        for (const auto i : util::range(meta.fmt->channel_count))
        {
            channel_decoders[i]->decode1(
                bit_reader, params[8], tmp, ath_table);
        }

        for (const auto i : util::range(8))
        {
            for (const auto j : util::range(meta.fmt->channel_count))
                channel_decoders[j]->decode2(bit_reader);

            for (const auto j : util::range(meta.fmt->channel_count))
            {
                channel_decoders[j]->decode3(
                    params[8], params[7], params[6] + params[5], params[4]);
            }

            for (const auto j : util::range(meta.fmt->channel_count))
            {
                channel_decoders[j]->decode4(
                    i, params[4] - params[5], params[5], params[6]);
            }

            for (const auto j : util::range(meta.fmt->channel_count))
                channel_decoders[j]->decode5(i);
        }
    }
}

bool HcaAudioDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> HcaAudioDecoder::decode_impl(File &file) const
{
    // TODO when testable: this should be customizable.
    const u32 ciph_key1 = 0x30DBE1AB;
    const u32 ciph_key2 = 0xCC554639;

    file.io.seek(6);
    static const u16 meta_size = file.io.read_u16_be();

    file.io.seek(0);
    auto meta = read_meta(file.io.read(meta_size));

    if (meta.loop->enabled)
        throw err::CorruptDataError("Loops are not supported");

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
    for (const auto i : util::range(8))
        params[i] = meta.comp->unk[i];
    if (params[0] != 1 || params[1] != 15)
        throw err::CorruptDataError("Unsupported decoder params");
    params[8] = ceil2(params[4] - (params[5] + params[6]), params[7]);

    const auto types = get_types(meta, params);
    std::vector<std::shared_ptr<ChannelDecoder>> channel_decoders;
    for (const auto i : util::range(channel_count))
    {
        auto channel_decoder = std::make_shared<ChannelDecoder>(
            types[i],
            params[5] + params[6],
            params[5] + ((types[i] != 2) ? params[6] : 0));
        channel_decoders.push_back(channel_decoder);
    }

    file.io.seek(meta.hca->data_offset);
    std::vector<s16> samples;
    samples.reserve(0x80 * 8 * channel_count * block_count);
    for (const auto b : util::range(block_count))
    {
        decode_block(
            meta,
            ath_table,
            channel_decoders,
            params,
            permutator.permute(file.io.read(block_size)));

        for (const auto i : util::range(8))
        for (const auto j : util::range(0x80))
        for (const auto k : util::range(channel_count))
        {
            const auto value = clamp(channel_decoders[k]->wave[i][j]);
            samples.push_back(static_cast<s16>(value * 0x7FFF));
        }
    }

    bstr raw_samples = bstr(
        reinterpret_cast<const char*>(&samples[0]),
        samples.size() * 2);

    const auto bits_per_sample = 16;
    const auto block_align = channel_count * bits_per_sample / 8;
    const auto byte_rate = sample_rate * block_align;

    auto output_file = std::make_unique<File>();
    output_file->io.write("RIFF"_b);
    output_file->io.write("\x00\x00\x00\x00"_b);
    output_file->io.write("WAVE"_b);
    output_file->io.write("fmt "_b);
    output_file->io.write_u32_le(16);
    output_file->io.write_u16_le(1);
    output_file->io.write_u16_le(channel_count);
    output_file->io.write_u32_le(sample_rate);
    output_file->io.write_u32_le(byte_rate);
    output_file->io.write_u16_le(block_align);
    output_file->io.write_u16_le(bits_per_sample);
    output_file->io.write("data"_b);
    output_file->io.write_u32_le(raw_samples.size());
    output_file->io.write(raw_samples);
    output_file->io.seek(4);
    output_file->io.write_u32_le(output_file->io.size() - 8);

    // TODO: loops

    output_file->name = file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::register_fmt<HcaAudioDecoder>("cri/hca");
