#include "fmt/wild_bug/wpx_decoder.h"
#include <map>
#include "err.h"
#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WPX\x1A"_b;

namespace
{
    struct WpxDecoderContext final
    {
        io::IO &io;
        io::BitReader &bit_reader;
        size_t quant_size;
        const std::array<size_t, 8> &offsets;
    };

    struct TranscriptionSpec final
    {
        size_t look_behind;
        size_t size;
    };

    struct ITranscriptionStrategy
    {
        virtual ~ITranscriptionStrategy() { }
        virtual TranscriptionSpec get_spec(WpxDecoderContext &context) = 0;
    };

    struct TranscriptionStrategy1 final : ITranscriptionStrategy
    {
        virtual TranscriptionSpec get_spec(WpxDecoderContext &context) override;
    };

    struct TranscriptionStrategy2 final : ITranscriptionStrategy
    {
        virtual TranscriptionSpec get_spec(WpxDecoderContext &context) override;
    };

    struct IRetrievalStrategy
    {
        IRetrievalStrategy(io::BitReader &bit_reader) { }
        virtual ~IRetrievalStrategy() { }
        virtual u8 fetch_byte(WpxDecoderContext &, const u8 *) = 0;
    };

    struct RetrievalStrategy1 final : IRetrievalStrategy
    {
        RetrievalStrategy1(io::BitReader &bit_reader);
        virtual u8 fetch_byte(WpxDecoderContext &, const u8 *) override;
        std::vector<u8> dict;
        std::vector<u8> table;
    };

    struct RetrievalStrategy2 final : IRetrievalStrategy
    {
        RetrievalStrategy2(io::BitReader &bit_reader);
        virtual u8 fetch_byte(WpxDecoderContext &, const u8 *) override;
        std::vector<u8> table;
    };

    struct RetrievalStrategy3 final : IRetrievalStrategy
    {
        RetrievalStrategy3(io::BitReader &bit_reader);
        virtual u8 fetch_byte(WpxDecoderContext &, const u8 *) override;
    };
}

static std::vector<u8> build_dict()
{
    std::vector<u8> dict(0x100 * 0x100);
    for (auto i : util::range(0x100))
    {
        u8 n = -1 - i;
        for (auto j : util::range(0x100))
            dict[0x100 * i + j] = n--;
    }
    return dict;
}

static std::vector<u8> build_table(io::BitReader &bit_reader)
{
    std::vector<u8> sizes(0x100);
    for (auto n : util::range(0, 0x100, 2))
    {
        sizes[n + 1] = bit_reader.get(4);
        sizes[n] = bit_reader.get(4);
    }

    std::vector<u8> table(0x10000);
    for (auto n : util::range(0x100))
    {
        auto size = sizes[n];
        if (!size)
            continue;
        u16 index = bit_reader.get(size);
        index <<= 15 - size;
        index &= 0x7FFF;
        table[2 * index] = size;
        table[2 * index + 1] = n;
    }
    return table;
}

static int find_table_index(
    const std::vector<u8> &table, io::BitReader &bit_reader)
{
    auto index = 0;
    u8 n = 0;
    auto mask = 0x4000;
    while (true)
    {
        n++;
        if (bit_reader.get(1))
            index |= mask;
        if (n == table[2 * index])
            break;
        if (!(mask >>= 1))
            throw err::CorruptDataError("Corrupt data");
    }
    return index;
}

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

RetrievalStrategy1::RetrievalStrategy1(io::BitReader &bit_reader)
    : IRetrievalStrategy(bit_reader)
{
    dict = build_dict();
    table = build_table(bit_reader);
}

u8 RetrievalStrategy1::fetch_byte(
    WpxDecoderContext &context, const u8 *output_ptr)
{
    auto start_pos = output_ptr[-context.quant_size] << 8;
    auto index = find_table_index(table, context.bit_reader);
    auto size = table[2 * index + 1];
    auto value = dict[start_pos + size];
    while (size)
    {
        dict[start_pos + size] = dict[start_pos + size - 1];
        size--;
    }
    dict[start_pos] = value;
    return value;
}

RetrievalStrategy2::RetrievalStrategy2(io::BitReader &bit_reader)
    : IRetrievalStrategy(bit_reader)
{
    table = build_table(bit_reader);
}

u8 RetrievalStrategy2::fetch_byte(
    WpxDecoderContext &context, const u8 *output_ptr)
{
    auto start_pos = output_ptr[-context.quant_size] << 8;
    auto index = find_table_index(table, context.bit_reader);
    return table[2 * index + 1];
}

RetrievalStrategy3::RetrievalStrategy3(io::BitReader &bit_reader)
    : IRetrievalStrategy(bit_reader)
{
}

u8 RetrievalStrategy3::fetch_byte(
    WpxDecoderContext &context, const u8 *output_ptr)
{
    return context.io.read_u8();
}

TranscriptionSpec TranscriptionStrategy1::get_spec(WpxDecoderContext &context)
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
        spec.look_behind = context.offsets[context.bit_reader.get(3)];
        spec.size = context.quant_size == 1 ? 2 : 1;
    }
    spec.size += read_count(context.bit_reader);
    return spec;
}

TranscriptionSpec TranscriptionStrategy2::get_spec(WpxDecoderContext &context)
{
    TranscriptionSpec spec;
    spec.look_behind = context.offsets[context.bit_reader.get(3)];
    spec.size = context.quant_size == 1 ? 2 : 1;
    spec.size += read_count(context.bit_reader);
    return spec;
}

struct WpxDecoder::Priv final
{
    Priv(io::IO &io);

    io::IO &io;
    std::string tag;
    std::map<u8, WpxSection> sections;
};

WpxDecoder::Priv::Priv(io::IO &io) : io(io)
{
}

WpxDecoder::WpxDecoder(io::IO &io) : p(new Priv(io))
{
    if (io.read(magic.size()) != magic)
        throw err::RecognitionError();

    io.seek(0x04);
    p->tag = io.read_to_zero(4).str();

    io.seek(0x0C);
    if (io.read_u8() != 1)
        throw err::CorruptDataError("Corrupt WPX header");

    io.seek(0x0E);
    auto section_count = io.read_u8();
    auto dir_size = io.read_u8();

    for (auto i : util::range(section_count))
    {
        auto id = io.read_u8();
        WpxSection section;
        section.data_format = io.read_u8();
        io.skip(2);
        section.offset = io.read_u32_le();
        section.size_orig = io.read_u32_le();
        section.size_comp = io.read_u32_le();
        p->sections[id] = section;
    }
}

WpxDecoder::~WpxDecoder()
{
}

std::string WpxDecoder::get_tag() const
{
    return p->tag;
}

bool WpxDecoder::has_section(u8 section_id) const
{
    return p->sections.find(section_id) != p->sections.end();
}

bstr WpxDecoder::read_plain_section(u8 section_id)
{
    auto section = p->sections.at(section_id);
    if ((section.data_format & 0x80) || !section.size_comp)
    {
        p->io.seek(section.offset);
        return p->io.read(section.size_orig);
    }
    throw err::CorruptDataError("Section is compressed");
}

bstr WpxDecoder::read_compressed_section(
    u8 section_id, u8 quant_size, const std::array<size_t, 8> &offsets)
{
    auto section = p->sections.at(section_id);
    if ((section.data_format & 0x80) || !section.size_comp)
        return read_plain_section(section_id);

    p->io.seek(section.offset);
    io::BufferedIO section_io(p->io);

    bstr output(section.size_orig);
    auto output_start = output.get<u8>();
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    for (auto i : util::range(quant_size))
        *output_ptr++ = section_io.read_u8();
    int remaining = section.size_orig - quant_size;
    section_io.seek((-quant_size & 3) + quant_size);
    io::BitReader bit_reader(section_io);

    std::unique_ptr<ITranscriptionStrategy> transcription_strategy;
    std::unique_ptr<IRetrievalStrategy> retrieval_strategy;

    if (section.data_format & 1)
    {
        if (section.data_format & 8)
            throw err::NotSupportedError("Unknown compression type");
        else
            transcription_strategy.reset(new TranscriptionStrategy1);
    }
    else
        transcription_strategy.reset(new TranscriptionStrategy2);

    if (section.data_format & 4)
        retrieval_strategy.reset(new RetrievalStrategy1(bit_reader));
    else if (section.data_format & 2)
        retrieval_strategy.reset(new RetrievalStrategy2(bit_reader));
    else
        retrieval_strategy.reset(new RetrievalStrategy3(bit_reader));

    WpxDecoderContext context
    {
        section_io,
        bit_reader,
        quant_size,
        offsets,
    };

    while (output_ptr < output_end)
    {
        if (context.bit_reader.get(1))
        {
            *output_ptr = retrieval_strategy->fetch_byte(context, output_ptr);
            output_ptr++;
        }
        else
        {
            auto spec = transcription_strategy->get_spec(context);
            auto source_ptr = &output_ptr[-spec.look_behind];
            if (source_ptr >= output_start)
                while (spec.size-- && output_ptr < output_end)
                    *output_ptr++ = *source_ptr++;
        }
    }

    return output;
}
