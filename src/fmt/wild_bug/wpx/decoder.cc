#include "fmt/wild_bug/wpx/decoder.h"
#include <map>
#include "err.h"
#include "fmt/wild_bug/wpx/transcription.h"
#include "fmt/wild_bug/wpx/retrieval.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::wild_bug::wpx;

static const bstr magic = "WPX\x1A"_b;

namespace
{
    struct Section final
    {
        u32 data_format;
        u32 offset;
        u32 size_orig;
        u32 size_comp;
    };
}

struct Decoder::Priv final
{
    Priv(io::IO &io);

    io::IO &io;
    std::string tag;
    std::map<u8, Section> sections;
};

Decoder::Priv::Priv(io::IO &io) : io(io)
{
}

Decoder::Decoder(io::IO &io) : p(new Priv(io))
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
        Section section;
        section.data_format = io.read_u8();
        io.skip(2);
        section.offset = io.read_u32_le();
        section.size_orig = io.read_u32_le();
        section.size_comp = io.read_u32_le();
        p->sections[id] = section;
    }
}

Decoder::~Decoder()
{
}

std::string Decoder::get_tag() const
{
    return p->tag;
}

bool Decoder::has_section(u8 section_id) const
{
    return p->sections.find(section_id) != p->sections.end();
}

bstr Decoder::read_plain_section(u8 section_id)
{
    auto section = p->sections.at(section_id);
    if ((section.data_format & 0x80) || !section.size_comp)
    {
        p->io.seek(section.offset);
        return p->io.read(section.size_orig);
    }
    throw err::CorruptDataError("Section is compressed");
}

bstr Decoder::read_compressed_section(
    u8 section_id, s8 quant_size, const std::array<size_t, 8> &offsets)
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

    std::unique_ptr<ITranscriptionStrategy> transcriptor;
    std::unique_ptr<IRetrievalStrategy> retriever;

    if (section.data_format & 1)
    {
        if (section.data_format & 8)
            throw err::NotSupportedError("Unknown compression type");
        else
            transcriptor.reset(
                new TranscriptionStrategy1(offsets, quant_size));
    }
    else
        transcriptor.reset(new TranscriptionStrategy2(offsets, quant_size));

    if (section.data_format & 4)
        retriever.reset(new RetrievalStrategy1(bit_reader, quant_size));
    else if (section.data_format & 2)
        retriever.reset(new RetrievalStrategy2(bit_reader, quant_size));
    else
        retriever.reset(new RetrievalStrategy3(bit_reader));

    DecoderContext context { section_io, bit_reader };

    while (output_ptr < output_end)
    {
        if (context.bit_reader.get(1))
        {
            *output_ptr = retriever->fetch_byte(context, output_ptr);
            output_ptr++;
        }
        else
        {
            auto spec = transcriptor->get_spec(context);
            auto source_ptr = &output_ptr[-spec.look_behind];
            if (source_ptr >= output_start)
                while (spec.size-- && output_ptr < output_end)
                    *output_ptr++ = *source_ptr++;
        }
    }

    return output;
}
