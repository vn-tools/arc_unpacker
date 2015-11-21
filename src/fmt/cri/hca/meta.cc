#include "fmt/cri/hca/meta.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cri::hca;

static void extend_meta(Meta &meta)
{
    if (!meta.comp)
    {
        if (!meta.dec)
        {
            throw err::CorruptDataError(
                "Neither 'comp' or 'dec' chunk was found");
        }
        // translate dec to comp chunk (apparently, they're equivalent)
        meta.comp = std::make_unique<CompChunk>();
        meta.comp->block_size = meta.dec->block_size;
        meta.comp->unk[0] = meta.dec->unk[0];
        meta.comp->unk[1] = meta.dec->unk[1];
        meta.comp->unk[2] = meta.dec->unk[2];
        meta.comp->unk[3] = meta.dec->unk[3];
        meta.comp->unk[4] = meta.dec->count[0] + 1;
        meta.comp->unk[5] = meta.dec->count[meta.dec->enable_count2] + 1;
        meta.comp->unk[6] = meta.comp->unk[5] - meta.comp->unk[5];
        meta.comp->unk[7] = 0;
    }

    if (!meta.ath)
    {
        meta.ath = std::make_unique<AthChunk>();
        meta.ath->type = meta.hca->version < 0x200 ? 1 : 0;
    }

    if (!meta.loop)
    {
        meta.loop = std::make_unique<LoopChunk>();
        meta.loop->start = 0;
        meta.loop->end = 0;
        meta.loop->repetitions = 0;
        meta.loop->unk = 0x400;
        meta.loop->enabled = false;
    }

    if (!meta.rva)
    {
        meta.rva = std::make_unique<RvaChunk>();
        meta.rva->volume = 1;
    }
}

Meta fmt::cri::hca::read_meta(const bstr &input)
{
    Meta out;
    io::MemoryStream stream(input);
    while (!stream.eof())
    {
        const bstr magic = stream.read(4);

        if (magic == "HCA\x00"_b)
        {
            out.hca = std::make_unique<HcaChunk>();
            out.hca->version = stream.read_u16_be();
            out.hca->data_offset = stream.read_u16_be();
        }

        else if (magic == "fmt\x00"_b)
        {
            out.fmt = std::make_unique<FmtChunk>();
            out.fmt->channel_count = stream.read_u8();
            out.fmt->sample_rate
                = (stream.read_u16_be() << 8) | stream.read_u8();
            out.fmt->block_count = stream.read_u32_be();
            stream.skip(4);
        }

        else if (magic == "dec\x00"_b)
        {
            out.dec = std::make_unique<DecChunk>();
            out.dec->block_size = stream.read_u16_be();
            out.dec->unk[0] = stream.read_u8();
            out.dec->unk[1] = stream.read_u8();
            out.dec->count[0] = stream.read_u8();
            out.dec->count[1] = stream.read_u8();
            const auto tmp = stream.read_u8();
            out.dec->unk[2] = tmp >> 4;
            out.dec->unk[3] = tmp & 0b1111;
            out.dec->enable_count2 = stream.read_u8();
        }

        else if (magic == "comp"_b)
        {
            out.comp = std::make_unique<CompChunk>();
            out.comp->block_size = stream.read_u16_be();
            for (const auto i : util::range(8))
                out.comp->unk[i] = stream.read_u8();
            stream.skip(2);
        }

        else if (magic == "vbr\x00"_b)
        {
            out.vbr = std::make_unique<VbrChunk>();
            out.vbr->unk[0] = stream.read_u16_be();
            out.vbr->unk[1] = stream.read_u16_be();
        }

        else if (magic == "ath\x00"_b)
        {
            out.ath = std::make_unique<AthChunk>();
            out.ath->type = stream.read_u16_le();
        }

        else if (magic == "loop"_b)
        {
            out.loop = std::make_unique<LoopChunk>();
            out.loop->start = stream.read_u32_be();
            out.loop->end = stream.read_u32_be();
            out.loop->repetitions = stream.read_u16_be();
            out.loop->unk = stream.read_u16_be();
            out.loop->enabled = true;
        }

        else if (magic == "ciph"_b)
        {
            out.ciph = std::make_unique<CiphChunk>();
            out.ciph->type = stream.read_u16_be();
        }

        else if (magic == "rva\x00"_b)
        {
            out.rva = std::make_unique<RvaChunk>();
            const auto tmp = stream.read(4);
            bstr reversed(4);
            reversed[0] = tmp[3];
            reversed[1] = tmp[2];
            reversed[2] = tmp[1];
            reversed[3] = tmp[0];
            out.rva->volume = *reversed.get<const float>();
        }

        else if (magic == "comm"_b)
        {
            out.comm = std::make_unique<CommChunk>();
            const auto tmp = stream.read_u32_be();
            out.comm->text = stream.read(tmp);
        }

        else if (magic == "pad\x00"_b)
            break;

        else
            throw err::NotSupportedError("Unknown chunk: " + magic.str());
    }

    extend_meta(out);
    return out;
}
