#include "fmt/wild_bug/wpx/transcription.h"
#include "fmt/wild_bug/wpx/common.h"

using namespace au;
using namespace au::fmt::wild_bug::wpx;

static u32 read_count(io::BitReader &bit_reader)
{
    auto n = 0;
    while (!bit_reader.get(1))
        n++;
    u32 ret = 1;
    while (n--)
    {
        ret <<= 1;
        ret += bit_reader.get(1);
    }
    return ret - 1;
}

TranscriptionStrategy1::TranscriptionStrategy1(
    const std::array<size_t, 8> &offsets, s8 quant_size)
    : offsets(offsets), quant_size(quant_size)
{
}

TranscriptionSpec TranscriptionStrategy1::get_spec(DecoderContext &context)
{
    TranscriptionSpec spec;
    if (context.bit_reader.get(1))
    {
        if (context.bit_reader.get(1))
        {
            spec.look_behind = context.io.read_u8() + 1;
            spec.size = 2;
        }
        else
        {
            spec.look_behind = context.io.read_u16_le() + 1;
            spec.size = 3;
        }
    }
    else
    {
        spec.look_behind = offsets[context.bit_reader.get(3)];
        spec.size = quant_size == 1 ? 2 : 1;
    }
    spec.size += read_count(context.bit_reader);
    return spec;
}

TranscriptionStrategy2::TranscriptionStrategy2(
    const std::array<size_t, 8> &offsets, s8 quant_size)
    : offsets(offsets), quant_size(quant_size)
{
}

TranscriptionSpec TranscriptionStrategy2::get_spec(DecoderContext &context)
{
    TranscriptionSpec spec;
    spec.look_behind = offsets[context.bit_reader.get(3)];
    spec.size = quant_size == 1 ? 2 : 1;
    spec.size += read_count(context.bit_reader);
    return spec;
}

TranscriptionSpec TranscriptionStrategy3::get_spec(DecoderContext &context)
{
    TranscriptionSpec spec;
    if (context.bit_reader.get(1))
    {
        spec.look_behind = context.io.read_u8() + 1;
        spec.size = 2;
    }
    else
    {
        spec.look_behind = context.io.read_u16_le() + 1;
        spec.size = 3;
    }
    spec.size += read_count(context.bit_reader);
    return spec;
}
