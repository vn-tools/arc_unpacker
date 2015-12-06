#include "fmt/wild_bug/wpx/decoder.h"
#include <map>
#include "err.h"
#include "fmt/wild_bug/wpx/retrieval.h"
#include "fmt/wild_bug/wpx/transcription.h"
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
    Priv(io::Stream &stream);

    io::Stream &stream;
    std::string tag;
    std::map<u8, Section> sections;
};

Decoder::Priv::Priv(io::Stream &stream) : stream(stream)
{
}

Decoder::Decoder(io::Stream &stream) : p(new Priv(stream))
{
    if (stream.read(magic.size()) != magic)
        throw err::RecognitionError();

    stream.seek(0x04);
    p->tag = stream.read_to_zero(4).str();

    stream.seek(0x0C);
    if (stream.read_u8() != 1)
        throw err::CorruptDataError("Corrupt WPX header");

    stream.seek(0x0E);
    auto section_count = stream.read_u8();
    auto dir_size = stream.read_u8();

    for (auto i : util::range(section_count))
    {
        auto id = stream.read_u8();
        Section section;
        section.data_format = stream.read_u8();
        stream.skip(2);
        section.offset = stream.read_u32_le();
        section.size_orig = stream.read_u32_le();
        section.size_comp = stream.read_u32_le();
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

const std::vector<u8> Decoder::get_sections() const
{
    std::vector<u8> ids;
    for (auto it : p->sections)
        ids.push_back(it.first);
    return ids;
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
        p->stream.seek(section.offset);
        return p->stream.read(section.size_orig);
    }
    throw err::CorruptDataError("Section is compressed");
}

bstr Decoder::read_compressed_section(u8 section_id)
{
    return read_compressed_section(section_id, -1, {0, 0, 0, 0, 0, 0, 0, 0});
}

bstr Decoder::read_compressed_section(
    u8 section_id, s8 quant_size, const std::array<size_t, 8> &offsets)
{
    auto section = p->sections.at(section_id);
    if ((section.data_format & 0x80) || !section.size_comp)
        return read_plain_section(section_id);

    p->stream.seek(section.offset);
    io::MemoryStream section_stream(p->stream);

    bstr output(section.size_orig);
    auto output_start = output.get<u8>();
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    bool use_plain_transcriptors = false;
    if (quant_size == -1)
    {
        quant_size = 1;
        use_plain_transcriptors = true;
    }

    for (auto i : util::range(quant_size))
        *output_ptr++ = section_stream.read_u8();
    int remaining = section.size_orig - quant_size;
    section_stream.seek((-quant_size & 3) + quant_size);
    io::BitReader bit_reader(section_stream);

    std::unique_ptr<ITranscriptionStrategy> transcriptor;
    if (use_plain_transcriptors)
    {
        if (section.data_format & 8)
            throw err::NotSupportedError("Unknown compression type");
        else
            transcriptor.reset(new TranscriptionStrategy3());
    }
    else
    {
        if (section.data_format & 1)
        {
            if (section.data_format & 8)
                throw err::NotSupportedError("Unknown compression type");
            else
                transcriptor.reset(new TranscriptionStrategy1(
                    offsets, quant_size));
        }
        else
            transcriptor.reset(new TranscriptionStrategy2(offsets, quant_size));
    }

    std::unique_ptr<IRetrievalStrategy> retriever;
    if (section.data_format & 4)
        retriever.reset(new RetrievalStrategy1(bit_reader, quant_size));
    else if (section.data_format & 2)
        retriever.reset(new RetrievalStrategy2(bit_reader));
    else
        retriever.reset(new RetrievalStrategy3(bit_reader));

    DecoderContext context {section_stream, bit_reader};
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
