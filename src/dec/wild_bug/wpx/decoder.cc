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

#include "dec/wild_bug/wpx/decoder.h"
#include <map>
#include "algo/range.h"
#include "dec/wild_bug/wpx/retrieval.h"
#include "dec/wild_bug/wpx/transcription.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::wild_bug::wpx;

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
    Priv(io::BaseByteStream &input_stream);

    io::BaseByteStream &input_stream;
    std::string tag;
    std::map<u8, Section> sections;
};

Decoder::Priv::Priv(io::BaseByteStream &input_stream)
    : input_stream(input_stream)
{
}

Decoder::Decoder(io::BaseByteStream &input_stream) : p(new Priv(input_stream))
{
    if (input_stream.read(magic.size()) != magic)
        throw err::RecognitionError();

    input_stream.seek(0x04);
    p->tag = input_stream.read_to_zero(4).str();

    input_stream.seek(0x0C);
    if (input_stream.read<u8>() != 1)
        throw err::CorruptDataError("Corrupt WPX header");

    input_stream.seek(0x0E);
    auto section_count = input_stream.read<u8>();
    auto dir_size = input_stream.read<u8>();

    for (const auto i : algo::range(section_count))
    {
        auto id = input_stream.read<u8>();
        Section section;
        section.data_format = input_stream.read<u8>();
        input_stream.skip(2);
        section.offset = input_stream.read_le<u32>();
        section.size_orig = input_stream.read_le<u32>();
        section.size_comp = input_stream.read_le<u32>();
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
    for (const auto it : p->sections)
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
        p->input_stream.seek(section.offset);
        return p->input_stream.read(section.size_orig);
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

    p->input_stream.seek(section.offset);
    io::MemoryByteStream section_stream(p->input_stream);

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

    for (const auto i : algo::range(quant_size))
        *output_ptr++ = section_stream.read<u8>();
    int remaining = section.size_orig - quant_size;
    section_stream.seek((-quant_size & 3) + quant_size);
    io::MsbBitStream bit_stream(section_stream);

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
        retriever.reset(new RetrievalStrategy1(bit_stream, quant_size));
    else if (section.data_format & 2)
        retriever.reset(new RetrievalStrategy2(bit_stream));
    else
        retriever.reset(new RetrievalStrategy3(bit_stream));

    DecoderContext context {section_stream, bit_stream};
    while (output_ptr < output_end)
    {
        if (context.bit_stream.read(1))
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
