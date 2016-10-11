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

#include "dec/entis/audio/lossy.h"
#include <cmath>
#include "algo/range.h"
#include "dec/entis/common/gamma_decoder.h"
#include "dec/entis/common/huffman_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis;
using namespace au::dec::entis::audio;

static const int min_dct_degree = 2;
static const int max_dct_degree = 12;

static const f64 pi = 3.141592653589;
static const f32 rcos_pi_4 = static_cast<f32>(std::cos(pi / 4.0));
static const f32 r2cos_pi_4 = 2.0f * rcos_pi_4;

static f32 dct_of_k2[2];       // = std::cos((2*i+1) / 8)
static f32 dct_of_k4[4];       // = std::cos((2*i+1) / 16)
static f32 dct_of_k8[8];       // = std::cos((2*i+1) / 32)
static f32 dct_of_k16[16];     // = std::cos((2*i+1) / 64)
static f32 dct_of_k32[32];     // = std::cos((2*i+1) / 128)
static f32 dct_of_k64[64];     // = std::cos((2*i+1) / 256)
static f32 dct_of_k128[128];   // = std::cos((2*i+1) / 512)
static f32 dct_of_k256[256];   // = std::cos((2*i+1) / 1024)
static f32 dct_of_k512[512];   // = std::cos((2*i+1) / 2048)
static f32 dct_of_k1024[1024]; // = std::cos((2*i+1) / 4096)
static f32 dct_of_k2048[2048]; // = std::cos((2*i+1) / 8192)

static f32 *dct_of_k_matrix[max_dct_degree] =
{
    nullptr,
    dct_of_k2,
    dct_of_k4,
    dct_of_k8,
    dct_of_k16,
    dct_of_k32,
    dct_of_k64,
    dct_of_k128,
    dct_of_k256,
    dct_of_k512,
    dct_of_k1024,
    dct_of_k2048
};

namespace
{
    struct EriSinCos final
    {
        f32 rsin;
        f32 rcos;
    };
}

struct LossyAudioDecoder::Priv final
{
    Priv(const MioHeader &header);
    ~Priv();

    void initialize_with_degree(const size_t subband_degree);

    bstr decode_dct(const MioChunk &chunk);
    bstr decode_dct_mss(const MioChunk &chunk);

    void decode_lead_block();
    void decode_internal_block(s16 *output_ptr, const size_t samples);
    void decode_post_block(s16 *output_ptr, const size_t samples);

    void decode_lead_block_mss();
    void decode_internal_block_mss(s16 *output_ptr, const size_t samples);
    void decode_post_block_mss(s16 *output_ptr, const size_t samples);

    void dequantumize(
        f32 *destination,
        const s32 *quantumized,
        const s32 weight_code,
        const int coefficient);

    const MioHeader &header;
    std::unique_ptr<common::BaseDecoder> decoder;

    size_t buf_size;
    std::unique_ptr<s32[]> buffer1;
    std::unique_ptr<s32[]> buffer2;
    std::unique_ptr<s8[]> buffer3;
    std::unique_ptr<u8[]> division_table;
    std::unique_ptr<u8[]> revolve_code_table;
    std::unique_ptr<s32[]> weight_code_table;
    std::unique_ptr<u32[]> coefficient_table;
    std::unique_ptr<f32[]> matrix_buf;
    std::unique_ptr<f32[]> internal_buf;
    std::unique_ptr<f32[]> work_buf;
    std::unique_ptr<f32[]> work_buf2;
    std::unique_ptr<f32[]> weight_table;
    std::unique_ptr<f32[]> last_dct;

    u8 *division_ptr;
    u8 *rev_code_ptr;
    s32 *weight_ptr;
    u32 *coefficient_ptr;
    s32 *source_ptr;
    f32 *last_dct_buf;
    size_t subband_degree;
    size_t degree_num;
    std::vector<EriSinCos> revolve_param;
    size_t frequency_point[7];
};

static void init_dct_of_k_matrix()
{
    for (const auto i : algo::range(1, max_dct_degree))
    {
        int n = 1 << i;
        f32 *dct_of_k = dct_of_k_matrix[i];
        f64 nr = pi / (4.0 * n);
        f64 dr = nr + nr;
        f64 ir = nr;
        for (const auto j : algo::range(n))
        {
            dct_of_k[j] = static_cast<f32>(std::cos(ir));
            ir += dr;
        }
    }
}

static int round32(const f32 r)
{
    return (r >= 0.0)
        ? static_cast<int>(floor(r + 0.5))
        : static_cast<int>(ceil(r - 0.5));
}

static void round32_array(
    s16 *output, const int step, const f32 *source, const size_t size)
{
    for (const auto i : algo::range(size))
    {
        int value = round32(*source++);
        if (value <= -0x8000)
            *output = -0x8000;
        else if (value >= 0x7FFF)
            *output = 0x7FFF;
        else
            *output = value;
        output += step;
    }
}

static void iplot(f32 *input, const size_t dct_degree)
{
    const auto degree_num = 1 << dct_degree;
    for (const auto i : algo::range(0, degree_num, 2))
    {
        const auto r1 = input[i];
        const auto r2 = input[i + 1];
        input[i + 0] = 0.5f * (r1 + r2);
        input[i + 1] = 0.5f * (r1 - r2);
    }
}

static void ilot(
    f32 *output,
    const f32 *input1,
    const f32 *input2,
    const size_t dct_degree)
{
    const auto degree_num = 1 << dct_degree;
    for (const auto i : algo::range(0, degree_num, 2))
    {
        const auto r1 = input1[i + 0];
        const auto r2 = input2[i + 1];
        output[i + 0] = r1 + r2;
        output[i + 1] = r1 - r2;
    }
}

static std::vector<EriSinCos> create_revolve_param(const size_t dct_degree)
{
    const signed int degree_num = 1 << dct_degree;

    int lc = 1;
    for (int n = degree_num / 2; n >= 8; n /= 8)
        ++lc;

    std::vector<EriSinCos> revolve_param(lc * 8);
    const f64 k = pi / (degree_num * 2);
    EriSinCos *revolve_param_ptr = &revolve_param[0];
    signed int step = 2;
    do
    {
        for (const auto i : algo::range(7))
        {
            f64 ws = 1.0;
            f64 a = 0.0;
            for (const auto j : algo::range(i))
            {
                a += step;
                ws = ws * revolve_param_ptr[j].rsin
                    + revolve_param_ptr[j].rcos * std::cos(a * k);
            }
            const f64 r = std::atan2(ws, std::cos((a + step) * k));
            revolve_param_ptr[i].rsin = static_cast<f32>(std::sin(r));
            revolve_param_ptr[i].rcos = static_cast<f32>(std::cos(r));
        }
        revolve_param_ptr += 7;
        step *= 8;
    }
    while (step < degree_num);
    return revolve_param;
}

static void revolve_2x2(
    f32 *buf1,
    f32 *buf2,
    const f32 rsin,
    const f32 rcos,
    const size_t step,
    const size_t size)
{
    for (const auto i : algo::range(size))
    {
        const f32 r1 = *buf1;
        const f32 r2 = *buf2;
        *buf1 = r1 * rcos - r2 * rsin;
        *buf2 = r1 * rsin + r2 * rcos;
        buf1 += step;
        buf2 += step;
    }
}

static void odd_givens_inverse_matrix(
    f32 *input,
    const std::vector<EriSinCos> &revolve_param,
    const size_t dct_degree)
{
    const auto degree_num = 1 << dct_degree;
    const auto *revolve_ptr = &revolve_param[0];
    auto index = 1;
    auto step = 2;
    auto lc = (degree_num / 2) / 8;
    while (true)
    {
        revolve_ptr += 7;
        index += step * 7;
        step *= 8;
        if (lc <= 8)
            break;
        lc /= 8;
    }
    auto k = index + step * (lc - 2);
    for (int j = lc - 2; j >= 0; j--)
    {
        const auto r1 = input[k];
        const auto r2 = input[k + step];
        input[k] = r1 * revolve_ptr[j].rcos + r2 * revolve_ptr[j].rsin;
        input[k + step] = r2 * revolve_ptr[j].rcos - r1 * revolve_ptr[j].rsin;
        k -= step;
    }
    while (true)
    {
        if (lc > (degree_num / 2) / 8)
            break;
        revolve_ptr -= 7;
        step /= 8;
        index -= step * 7;
        for (const auto i : algo::range(lc))
        {
            k = i * (step * 8) + index + step * 6;
            for (int j = 6; j >= 0; j--)
            {
                const auto r1 = input[k];
                const auto r2 = input[k + step];
                input[k] = r1 * revolve_ptr[j].rcos + r2 * revolve_ptr[j].rsin;
                input[k + step] =
                    r2 * revolve_ptr[j].rcos - r1 * revolve_ptr[j].rsin;
                k -= step;
            }
        }
        lc *= 8;
    }
}

static void dct(
    f32 *output,
    const size_t output_interval,
    f32 *input,
    f32 *work_buf,
    const size_t dct_degree)
{
    if (dct_degree < min_dct_degree || dct_degree > max_dct_degree)
        throw std::logic_error("DCT degree out of bounds");

    if (dct_degree == min_dct_degree)
    {
        f32 r32_buf[4];
        r32_buf[0] = input[0] + input[3];
        r32_buf[2] = input[0] - input[3];
        r32_buf[1] = input[1] + input[2];
        r32_buf[3] = input[1] - input[2];
        output[output_interval * 0] = (r32_buf[0] + r32_buf[1]) * 0.5f;
        output[output_interval * 2] = (r32_buf[0] - r32_buf[1]) *  rcos_pi_4;
        r32_buf[2] = dct_of_k2[0] * r32_buf[2];
        r32_buf[3] = dct_of_k2[1] * r32_buf[3];
        r32_buf[0] = (r32_buf[2] + r32_buf[3]);
        r32_buf[1] = (r32_buf[2] - r32_buf[3]) * r2cos_pi_4;
        r32_buf[1] -= r32_buf[0];
        output[output_interval * 1] = r32_buf[0];
        output[output_interval * 3] = r32_buf[1];
        return;
    }

    const auto degree_num = 1 << dct_degree;
    const auto half_degree = degree_num >> 1;
    for (const auto i : algo::range(half_degree))
    {
        work_buf[i] = input[i] + input[degree_num - i - 1];
        work_buf[i + half_degree] = input[i] - input[degree_num - i - 1];
    }
    const auto output_step = output_interval << 1;
    dct(output, output_step, work_buf, input, dct_degree - 1);
    const auto dct_of_k = dct_of_k_matrix[dct_degree - 1];
    input = work_buf + half_degree;
    output += output_interval;
    for (const auto i : algo::range(half_degree))
        input[i] *= dct_of_k[i];
    dct(output, output_step, input, work_buf, dct_degree - 1);
    for (const auto i : algo::range(half_degree))
        output[i * output_step] += output[i * output_step];
    for (const auto i : algo::range(1, half_degree))
        output[i * output_step] -= output[(i - 1) * output_step];
}

static void idct(
    f32 *output,
    f32 *input,
    const size_t input_interval,
    f32 *work_buf,
    const size_t dct_degree)
{
    if (dct_degree < min_dct_degree || dct_degree > max_dct_degree)
        throw std::logic_error("DCT degree out of bounds");

    if (dct_degree == min_dct_degree)
    {
        f32 r32_buf1[2];
        f32 r32_buf2[4];
        r32_buf1[0] = input[0];
        r32_buf1[1] = rcos_pi_4 * input[input_interval * 2];
        r32_buf2[0] = r32_buf1[0] + r32_buf1[1];
        r32_buf2[1] = r32_buf1[0] - r32_buf1[1];
        r32_buf1[0] = dct_of_k2[0] * input[input_interval];
        r32_buf1[1] = dct_of_k2[1] * input[input_interval * 3];
        r32_buf2[2] = r32_buf1[0] + r32_buf1[1];
        r32_buf2[3] = r2cos_pi_4 * (r32_buf1[0] - r32_buf1[1]);
        r32_buf2[3] -= r32_buf2[2];
        output[0] = r32_buf2[0] + r32_buf2[2];
        output[3] = r32_buf2[0] - r32_buf2[2];
        output[1] = r32_buf2[1] + r32_buf2[3];
        output[2] = r32_buf2[1] - r32_buf2[3];
        return;
    }

    const size_t degree_num = 1 << dct_degree;
    const size_t half_degree = degree_num >> 1;
    const size_t input_step = input_interval << 1;
    idct(output, input, input_step, work_buf, dct_degree - 1);
    const f32 *dct_of_k = dct_of_k_matrix[dct_degree - 1];
    const f32 *odd_input = input + input_interval;
    f32 *odd_output = output + half_degree;
    for (const auto i : algo::range(half_degree))
        work_buf[i] = odd_input[i * input_step] * dct_of_k[i];
    dct(odd_output, 1, work_buf, work_buf + half_degree, dct_degree - 1);
    for (const auto i : algo::range(half_degree))
        odd_output[i] += odd_output[i];
    for (const auto i : algo::range(1, half_degree))
        odd_output[i] -= odd_output[i - 1];
    f32 r32_buf[4];
    for (const auto i : algo::range(half_degree >> 1))
    {
        r32_buf[0] = output[i] + output[half_degree + i];
        r32_buf[3] = output[i] - output[half_degree + i];
        r32_buf[1] = output[half_degree - 1 - i] + output[degree_num - 1 - i];
        r32_buf[2] = output[half_degree - 1 - i] - output[degree_num - 1 - i];
        output[i] = r32_buf[0];
        output[half_degree - 1 - i] = r32_buf[1];
        output[half_degree + i] = r32_buf[2];
        output[degree_num - 1 - i] = r32_buf[3];
    }
}

LossyAudioDecoder::Priv::Priv(const MioHeader &header) : header(header)
{
    if ((header.channel_count != 1) && (header.channel_count != 2))
        throw err::UnsupportedChannelCountError(header.channel_count);
    if (header.bits_per_sample != 16)
        throw err::UnsupportedBitDepthError(header.bits_per_sample);

    if ((header.subband_degree < 8) || (header.subband_degree > max_dct_degree))
        throw err::CorruptDataError("Unexpected subband degree");

    if (header.lapped_degree != 1)
        throw err::CorruptDataError("Unexpected lapped degree");

    buf_size = 0;

    const auto subband_size = 1 << header.subband_degree;
    const auto subband_size_total = header.channel_count * subband_size;
    buffer1 = std::make_unique<s32[]>(subband_size_total);
    matrix_buf = std::make_unique<f32[]>(subband_size_total);
    internal_buf = std::make_unique<f32[]>(subband_size_total);
    work_buf = std::make_unique<f32[]>(subband_size);
    work_buf2 = std::make_unique<f32[]>(subband_size);
    weight_table = std::make_unique<f32[]>(subband_size);

    const auto blockset_samples = header.channel_count << header.subband_degree;
    const auto lapped_samples = blockset_samples * header.lapped_degree;
    if (lapped_samples > 0)
    {
        last_dct = std::make_unique<f32[]>(lapped_samples);
        for (const auto i : algo::range(lapped_samples))
            last_dct[i] = 0.0f;
    }
    initialize_with_degree(header.subband_degree);
}

LossyAudioDecoder::Priv::~Priv()
{
}

void LossyAudioDecoder::Priv::initialize_with_degree(
    const size_t subband_degree)
{
    revolve_param = create_revolve_param(subband_degree);
    static const int freq_width[7] = {-6, -6, -5, -4, -3, -2, -1};
    auto j = 0;
    for (const auto i : algo::range(7))
    {
        const int frequency_width = 1 << (subband_degree + freq_width[i]);
        frequency_point[i] = j + (frequency_width / 2);
        j += frequency_width;
    }
    this->subband_degree = subband_degree;
    degree_num = 1 << subband_degree;
}

void LossyAudioDecoder::Priv::dequantumize(
    f32 *destination,
    const s32 *quantumized,
    const s32 weight_code,
    const int coefficient)
{
    const f64 matrix_scale = sqrt(2.0 / degree_num);
    const f64 coefficient_ratio = matrix_scale * coefficient;
    f64 avg_ratio[7];
    for (const auto i : algo::range(6))
    {
        const auto power = (((weight_code >> (i * 5)) & 0x1F) - 15) * 0.5;
        avg_ratio[i] = 1.0 / pow(2.0, power);
    }
    avg_ratio[6] = 1.0;

    size_t pos = 0;
    while (pos < frequency_point[0])
        weight_table[pos++] = static_cast<f32>(avg_ratio[0]);
    for (const auto j : algo::range(1, 7))
    {
        const f64 a = avg_ratio[j - 1];
        const f64 k = (avg_ratio[j] - a)
            / (frequency_point[j] - frequency_point[j - 1]);
        while (pos < frequency_point[j])
        {
            const auto tmp = k * (pos - frequency_point[j - 1]) + a;
            weight_table[pos++] = static_cast<f32>(tmp);
        }
    }
    while (pos < degree_num)
        weight_table[pos++] = static_cast<f32>(avg_ratio[6]);

    const f32 odd_weight_ratio = (((weight_code >> 30) & 3) + 2) / 2.0f;
    for (const auto i : algo::range(15, degree_num, 16))
        weight_table[i] *= odd_weight_ratio;

    weight_table[degree_num - 1] = static_cast<f32>(coefficient);

    for (const auto i : algo::range(degree_num))
        weight_table[i] = 1.0f / weight_table[i];
    for (const auto i : algo::range(degree_num))
    {
        destination[i] = static_cast<f32>(
            coefficient_ratio * weight_table[i] * quantumized[i]);
    }
}

void LossyAudioDecoder::Priv::decode_lead_block()
{
    const auto weight_code = *weight_ptr++;
    const auto coefficient = *coefficient_ptr++;
    const auto half_degree = degree_num / 2;
    for (const auto i : algo::range(half_degree))
    {
        buffer1[i * 2] = 0;
        buffer1[i * 2 + 1] = *source_ptr++;
    }
    dequantumize(last_dct_buf, buffer1.get(), weight_code, coefficient);
    odd_givens_inverse_matrix(last_dct_buf, revolve_param, subband_degree);
    for (const auto i : algo::range(0, degree_num, 2))
        last_dct_buf[i] = last_dct_buf[i + 1];
    iplot(last_dct_buf, subband_degree);
}

void LossyAudioDecoder::Priv::decode_internal_block(
    s16 *output_ptr, const size_t samples)
{
    const auto weight_code = *weight_ptr++;
    const auto coefficient = *coefficient_ptr++;
    dequantumize(matrix_buf.get(), source_ptr, weight_code, coefficient);
    source_ptr += degree_num;
    odd_givens_inverse_matrix(matrix_buf.get(), revolve_param, subband_degree);
    iplot(matrix_buf.get(), subband_degree);
    ilot(work_buf.get(), last_dct_buf, matrix_buf.get(), subband_degree);
    for (const auto i : algo::range(degree_num))
    {
        last_dct_buf[i] = matrix_buf[i];
        matrix_buf[i] = work_buf[i];
    }
    idct(
        internal_buf.get(),
        matrix_buf.get(),
        1,
        work_buf.get(),
        subband_degree);
    round32_array(
        output_ptr, header.channel_count, internal_buf.get(), samples);
}

void LossyAudioDecoder::Priv::decode_post_block(
    s16 * output_ptr, const size_t samples)
{
    const auto weight_code = *weight_ptr++;
    const auto coefficient = *coefficient_ptr++;
    const auto half_degree = degree_num / 2;
    for (const auto i : algo::range(half_degree))
    {
        buffer1[i * 2] = 0;
        buffer1[i * 2 + 1] = *source_ptr++;
    }
    dequantumize(matrix_buf.get(), buffer1.get(), weight_code, coefficient);
    odd_givens_inverse_matrix(matrix_buf.get(), revolve_param, subband_degree);
    for (const auto i : algo::range(0, degree_num, 2))
        matrix_buf[i] = -matrix_buf[i + 1];
    iplot(matrix_buf.get(), subband_degree);
    ilot(work_buf.get(), last_dct_buf, matrix_buf.get(), subband_degree);
    for (const auto i : algo::range(degree_num))
        matrix_buf[i] = work_buf[i];
    idct(
        internal_buf.get(),
        matrix_buf.get(),
        1,
        work_buf.get(),
        subband_degree);
    round32_array(
        output_ptr, header.channel_count, internal_buf.get(), samples);
}

bstr LossyAudioDecoder::Priv::decode_dct(const MioChunk &chunk)
{
    const auto degree_width = 1 << header.subband_degree;
    const auto sample_count
        = (chunk.sample_count + degree_width - 1) & ~(degree_width - 1);
    const auto subband_count = sample_count >> header.subband_degree;
    const auto channel_count = header.channel_count;
    const auto all_sample_count = sample_count * channel_count;
    const auto all_subband_count = subband_count * channel_count;

    if (all_sample_count > buf_size)
    {
        buffer2 = std::make_unique<s32[]>(all_sample_count);
        buffer3 = std::make_unique<s8[]>(all_sample_count * 2);
        division_table = std::make_unique<u8[]>(all_subband_count);
        revolve_code_table = std::make_unique<u8[]>(all_subband_count * 5);
        weight_code_table = std::make_unique<s32[]>(all_subband_count * 5);
        coefficient_table = std::make_unique<u32[]>(all_subband_count * 5);
        buf_size = all_sample_count;
    }

    if (decoder->bit_stream->read(1))
        throw err::CorruptDataError("Expected 0 bit");

    const auto last_division = std::make_unique<int[]>(channel_count);
    division_ptr = division_table.get();
    weight_ptr = weight_code_table.get();
    coefficient_ptr = coefficient_table.get();

    for (const auto i : algo::range(channel_count))
        last_division[i] = -1;

    for (const auto i : algo::range(subband_count))
    {
        for (const auto j : algo::range(channel_count))
        {
            const int division_code = decoder->bit_stream->read(2);
            *division_ptr++ = division_code;
            if (division_code != last_division[j])
            {
                if (i)
                {
                    *weight_ptr++ = decoder->bit_stream->read(32);
                    *coefficient_ptr++ = decoder->bit_stream->read(16);
                }
                last_division[j] = division_code;
            }
            const auto division_count = 1 << division_code;
            for (const auto k : algo::range(division_count))
            {
                *weight_ptr++ = decoder->bit_stream->read(32);
                *coefficient_ptr++ = decoder->bit_stream->read(16);
            }
        }
    }
    if (subband_count)
    {
        for (const auto i : algo::range(channel_count))
        {
            *weight_ptr++ = decoder->bit_stream->read(32);
            *coefficient_ptr++ = decoder->bit_stream->read(16);
        }
    }

    if (decoder->bit_stream->read(1))
        throw err::CorruptDataError("Expected 0 bit");

    if (chunk.initial || header.architecture == common::Architecture::Nemesis)
        decoder->reset();

    if (header.architecture != common::Architecture::Nemesis)
    {
        decoder->decode(
            reinterpret_cast<u8*>(buffer3.get()), all_sample_count * 2);

        const auto *hi_buf = buffer3.get();
        const auto *lo_buf = buffer3.get() + all_sample_count;
        for (const auto i : algo::range(degree_width))
        {
            auto quantumized_ptr = buffer2.get() + i;
            for (const auto j : algo::range(all_subband_count))
            {
                const s32 low = *lo_buf++;
                const s32 high = *hi_buf++ ^ (low >> 8);
                *quantumized_ptr = (low & 0xFF) | (high << 8);
                quantumized_ptr += degree_width;
            }
        }
    }
    else
        throw err::NotSupportedError("Unsupported architecture");

    bstr output(all_sample_count * sizeof(s16));
    auto output_ptrs = std::make_unique<s16*[]>(channel_count);
    auto samples_left = std::make_unique<size_t[]>(channel_count);
    division_ptr = division_table.get();
    weight_ptr = weight_code_table.get();
    coefficient_ptr = coefficient_table.get();
    source_ptr = buffer2.get();
    for (const auto i : algo::range(channel_count))
    {
        last_division[i] = -1;
        samples_left[i] = chunk.sample_count;
        output_ptrs[i] = output.get<s16>() + i;
    }

    int current_division = -1;
    for (const auto i : algo::range(subband_count))
    for (const auto j : algo::range(channel_count))
    {
        const auto division_code = *division_ptr++;
        const auto division_count = 1 << division_code;
        int channel_step = degree_width * header.lapped_degree * j;
        last_dct_buf = last_dct.get() + channel_step;
        auto lead_block = false;
        if (last_division[j] != division_code)
        {
            if (i)
            {
                if (current_division != last_division[j])
                {
                    initialize_with_degree
                        (header.subband_degree - last_division[j]);
                    current_division = last_division[j];
                }
                const auto samples_to_process
                    = std::min(samples_left[j], degree_num);
                decode_post_block(output_ptrs[j], samples_to_process);
                samples_left[j] -= samples_to_process;
                output_ptrs[j] += samples_to_process * channel_count;
            }
            last_division[j] = division_code;
            lead_block = true;
        }
        if (current_division != division_code)
        {
            initialize_with_degree(header.subband_degree - division_code);
            current_division = division_code;
        }
        for (const auto k : algo::range(division_count))
        {
            if (lead_block)
            {
                decode_lead_block();
                lead_block = false;
            }
            else
            {
                const auto samples_to_process
                    = std::min(samples_left[j], degree_num);
                decode_internal_block(output_ptrs[j], samples_to_process);
                samples_left[j] -= samples_to_process;
                output_ptrs[j] += samples_to_process * channel_count;
            }
        }
    }

    if (subband_count)
    {
        for (const auto i : algo::range(channel_count))
        {
            const int channel_step = degree_width * header.lapped_degree * i;
            last_dct_buf = last_dct.get() + channel_step;
            if (current_division != last_division[i])
            {
                initialize_with_degree
                    (header.subband_degree - last_division[i]);
                current_division = last_division[i];
            }
            const auto samples_to_process
                = std::min(samples_left[i], degree_num);
            decode_post_block(output_ptrs[i], samples_to_process);
            samples_left[i] -= samples_to_process;
            output_ptrs[i] += samples_to_process * channel_count;
        }
    }

    return output;
}

void LossyAudioDecoder::Priv::decode_lead_block_mss()
{
    const auto half_degree = degree_num / 2;
    const auto weight_code = *weight_ptr++;
    const auto coefficient = *coefficient_ptr++;
    auto lap_buf = last_dct.get();
    for (const auto i : algo::range(2))
    {
        for (const auto j : algo::range(half_degree))
        {
            buffer1[j * 2] = 0;
            buffer1[j * 2 + 1] = *source_ptr++;
        }
        dequantumize(lap_buf, buffer1.get(), weight_code, coefficient);
        lap_buf += degree_num;
    }
    const auto rev_code = *rev_code_ptr++;
    auto lap_buf1 = last_dct.get();
    auto lap_buf2 = last_dct.get() + degree_num;
    const auto rsin = static_cast<f32>(std::sin(rev_code * pi / 8));
    const auto rcos = static_cast<f32>(std::cos(rev_code * pi / 8));
    revolve_2x2(lap_buf1, lap_buf2, rsin, rcos, 1, degree_num);
    lap_buf = last_dct.get();
    for (const auto i : algo::range(2))
    {
        odd_givens_inverse_matrix(lap_buf, revolve_param, subband_degree);
        for (const auto j : algo::range(0, degree_num, 2))
            lap_buf[j] = lap_buf[j + 1];
        iplot(lap_buf, subband_degree);
        lap_buf += degree_num;
    }
}

void LossyAudioDecoder::Priv::decode_post_block_mss(
    s16 *output_ptr, size_t samples)
{
    auto matrix_ptr = matrix_buf.get();
    auto lap_buf = last_dct.get();
    const auto half_degree = degree_num / 2;
    const auto weight_code = *weight_ptr++;
    const auto coefficient = *coefficient_ptr++;
    for (const auto i : algo::range(2))
    {
        for (const auto j : algo::range(half_degree))
        {
            buffer1[j * 2] = 0;
            buffer1[j * 2 + 1] = *source_ptr++;
        }
        dequantumize(matrix_ptr, buffer1.get(), weight_code, coefficient);
        matrix_ptr += degree_num;
    }
    const auto rev_code = *rev_code_ptr++;
    auto matrix_ptr1 = matrix_buf.get();
    auto matrix_ptr2 = matrix_buf.get() + degree_num;
    const auto rsin = static_cast<f32>(std::sin(rev_code * pi / 8));
    const auto rcos = static_cast<f32>(std::cos(rev_code * pi / 8));
    revolve_2x2(matrix_ptr1, matrix_ptr2, rsin, rcos, 1, degree_num);
    matrix_ptr = matrix_buf.get();
    for (const auto i : algo::range(2))
    {
        odd_givens_inverse_matrix(matrix_ptr, revolve_param, subband_degree);
        for (const auto j : algo::range(0, degree_num, 2))
            matrix_ptr[j] = -matrix_ptr[j + 1];
        iplot(matrix_ptr, subband_degree);
        ilot(work_buf.get(), lap_buf, matrix_ptr, subband_degree);
        for (const auto j : algo::range(degree_num))
            matrix_ptr[j] = work_buf[j];
        idct(internal_buf.get(), matrix_ptr, 1, work_buf.get(), subband_degree);
        round32_array(output_ptr + i, 2, internal_buf.get(), samples);
        lap_buf += degree_num;
        matrix_ptr += degree_num;
    }
}

void LossyAudioDecoder::Priv::decode_internal_block_mss(
    s16 *output_ptr, size_t samples)
{
    auto matrix_ptr = matrix_buf.get();
    auto lap_buf = last_dct.get();
    const auto weight_code = *weight_ptr++;
    const auto coefficient = *coefficient_ptr++;
    for (const auto i : algo::range(2))
    {
        dequantumize(matrix_ptr, source_ptr, weight_code, coefficient);
        source_ptr += degree_num;
        matrix_ptr += degree_num;
    }

    const int rev_code = *rev_code_ptr++;
    const int rev_code1 = (rev_code >> 2) & 0x03;
    const int rev_code2 = rev_code & 0x03;

    f32 rsin, rcos;
    f32 *matrix_ptr1 = matrix_buf.get();
    f32 *matrix_ptr2 = matrix_buf.get() + degree_num;
    rsin = static_cast<f32>(std::sin(rev_code1 * pi / 8));
    rcos = static_cast<f32>(std::cos(rev_code1 * pi / 8));
    revolve_2x2(matrix_ptr1, matrix_ptr2, rsin, rcos, 2, degree_num / 2);
    rsin = static_cast<f32>(std::sin(rev_code2 * pi / 8));
    rcos = static_cast<f32>(std::cos(rev_code2 * pi / 8));
    revolve_2x2(matrix_ptr1+1, matrix_ptr2+1, rsin, rcos, 2, degree_num / 2);

    matrix_ptr = matrix_buf.get();
    for (const auto i : algo::range(2))
    {
        odd_givens_inverse_matrix(matrix_ptr, revolve_param, subband_degree);
        iplot(matrix_ptr, subband_degree);
        ilot(work_buf.get(), lap_buf, matrix_ptr, subband_degree);
        for (const auto j : algo::range(degree_num))
        {
            lap_buf[j] = matrix_ptr[j];
            matrix_ptr[j] = work_buf[j];
        }
        idct(internal_buf.get(), matrix_ptr, 1, work_buf.get(), subband_degree);
        round32_array(output_ptr + i, 2, internal_buf.get(), samples);
        matrix_ptr += degree_num;
        lap_buf += degree_num;
    }
}

bstr LossyAudioDecoder::Priv::decode_dct_mss(const MioChunk &chunk)
{
    const auto degree_width = 1 << header.subband_degree;
    const auto sample_count
        = (chunk.sample_count + degree_width - 1) & ~(degree_width - 1);
    const auto subband_count = sample_count >> header.subband_degree;
    const auto channel_count = header.channel_count;
    const auto all_sample_count = sample_count * channel_count;
    const auto all_subband_count = subband_count;

    if (all_sample_count > buf_size)
    {
        buffer2 = std::make_unique<s32[]>(all_sample_count);
        buffer3 = std::make_unique<s8[]>(all_sample_count * 2);
        division_table = std::make_unique<u8[]>(all_subband_count);
        revolve_code_table = std::make_unique<u8[]>(all_subband_count * 10);
        weight_code_table = std::make_unique<s32[]>(all_subband_count * 10);
        coefficient_table = std::make_unique<u32[]>(all_subband_count * 10);
        buf_size = all_sample_count;
    }

    if (decoder->bit_stream->read(1))
        throw err::CorruptDataError("Expected 0 bit");

    int last_division_code = -1;
    division_ptr = division_table.get();
    rev_code_ptr = revolve_code_table.get();
    weight_ptr = weight_code_table.get();
    coefficient_ptr = coefficient_table.get();

    for (const auto i : algo::range(subband_count))
    {
        const int division_code = decoder->bit_stream->read(2);
        *division_ptr++ = division_code;

        bool lead_block = false;
        if (division_code != last_division_code)
        {
            if (i)
            {
                *rev_code_ptr++ = decoder->bit_stream->read(2);
                *weight_ptr++ = decoder->bit_stream->read(32);
                *coefficient_ptr++ = decoder->bit_stream->read(16);
            }
            lead_block = true;
            last_division_code = division_code;
        }

        const auto division_count = 1 << division_code;
        for (const auto k : algo::range(division_count))
        {
            if (lead_block)
            {
                *rev_code_ptr++ = decoder->bit_stream->read(2);
                lead_block = false;
            }
            else
            {
                *rev_code_ptr++ = decoder->bit_stream->read(4);
            }
            *weight_ptr++ = decoder->bit_stream->read(32);
            *coefficient_ptr++ = decoder->bit_stream->read(16);
        }
    }
    if (subband_count)
    {
        *rev_code_ptr++ = decoder->bit_stream->read(2);
        *weight_ptr++ = decoder->bit_stream->read(32);
        *coefficient_ptr++ = decoder->bit_stream->read(16);
    }

    if (decoder->bit_stream->read(1))
        throw err::CorruptDataError("Expected 0 bit");

    if (chunk.initial || header.architecture == common::Architecture::Nemesis)
        decoder->reset();

    if (header.architecture != common::Architecture::Nemesis)
    {
        decoder->decode(
            reinterpret_cast<u8*>(buffer3.get()), all_sample_count * 2);

        const auto *hi_buf = buffer3.get();
        const auto *lo_buf = buffer3.get() + all_sample_count;
        for (const auto i : algo::range(degree_width * 2))
        {
            auto quantumized_ptr = buffer2.get() + i;
            for (const auto j : algo::range(all_subband_count))
            {
                const s32 low = *lo_buf++;
                const s32 high = *hi_buf++ ^ (low >> 8);
                *quantumized_ptr = (low & 0xFF) | (high << 8);
                quantumized_ptr += degree_width * 2;
            }
        }
    }
    else
        throw err::NotSupportedError("Unsupported architecture");

    bstr output(all_sample_count * sizeof(s16));
    size_t samples_left = chunk.sample_count;
    s16 *output_ptr = output.get<s16>();

    last_division_code = -1;
    division_ptr = division_table.get();
    rev_code_ptr = revolve_code_table.get();
    weight_ptr = weight_code_table.get();
    coefficient_ptr = coefficient_table.get();
    source_ptr = buffer2.get();

    for (const auto i : algo::range(subband_count))
    {
        const auto division_code = *division_ptr++;
        const auto division_count = 1 << division_code;
        bool lead_block = false;

        if (last_division_code != division_code)
        {
            if (i)
            {
                const auto samples_to_process
                    = std::min(samples_left, degree_num);
                decode_post_block_mss(output_ptr, samples_to_process);
                samples_left -= samples_to_process;
                output_ptr += samples_to_process * channel_count;
            }
            initialize_with_degree(header.subband_degree - division_code);
            last_division_code = division_code;
            lead_block = true;
        }

        for (const auto k : algo::range(division_count))
        {
            if (lead_block)
            {
                decode_lead_block_mss();
                lead_block = false;
            }
            else
            {
                const auto samples_to_process
                    = std::min(samples_left, degree_num);
                decode_internal_block_mss(output_ptr, samples_to_process);
                samples_left -= samples_to_process;
                output_ptr += samples_to_process * channel_count;
            }
        }
    }

    if (subband_count)
    {
        const auto samples_to_process = std::min(samples_left, degree_num);
        decode_post_block_mss(output_ptr, samples_to_process);
        samples_left -= samples_to_process;
        output_ptr += samples_to_process * channel_count;
    }

    return output;
}

LossyAudioDecoder::LossyAudioDecoder(const MioHeader &header)
    : p(new Priv(header))
{
    init_dct_of_k_matrix();
    if (header.architecture == common::Architecture::RunLengthGamma)
    {
        // this is nonsense but hey, I just reimplement stuff
        // p->decoder = std::make_unique<common::GammaDecoder>();
        p->decoder = std::make_unique<common::HuffmanDecoder>();
    }
    else if (header.architecture == common::Architecture::RunLengthHuffman)
        p->decoder = std::make_unique<common::HuffmanDecoder>();
    else
        throw err::NotSupportedError("Unsupported architecture");
}

LossyAudioDecoder::~LossyAudioDecoder()
{
}

bstr LossyAudioDecoder::process_chunk(const MioChunk &chunk)
{
    p->decoder->set_input(chunk.data);
    if (p->header.channel_count != 2
        || p->header.transformation == common::Transformation::Lot)
    {
        return p->decode_dct(chunk);
    }
    return p->decode_dct_mss(chunk);
}
