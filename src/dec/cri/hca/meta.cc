#include "dec/cri/hca/meta.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::cri::hca;

static void extend_meta(Meta &meta)
{
    if (!meta.ath)
    {
        meta.ath = std::make_unique<AthChunk>();
        meta.ath->type = meta.hca->version < 0x200 ? 1 : 0;
    }

    if (!meta.rva)
    {
        meta.rva = std::make_unique<RvaChunk>();
        meta.rva->volume = 1;
    }
}

Meta dec::cri::hca::read_meta(const bstr &input)
{
    Meta out;
    io::MemoryStream stream(input);
    while (!stream.eof())
    {
        const bstr magic = stream.read(4);

        if (magic == "HCA\x00"_b)
        {
            out.hca = std::make_unique<HcaChunk>();
            out.hca->version = stream.read_be<u16>();
            out.hca->data_offset = stream.read_be<u16>();
        }

        else if (magic == "fmt\x00"_b)
        {
            out.fmt = std::make_unique<FmtChunk>();
            out.fmt->channel_count = stream.read<u8>();
            out.fmt->sample_rate
                = (stream.read_be<u16>() << 8) | stream.read<u8>();
            out.fmt->block_count = stream.read_be<u32>();
            stream.skip(4);
        }

        else if (magic == "dec\x00"_b)
        {
            out.comp = std::make_unique<CompChunk>();
            out.comp->block_size = stream.read_be<u16>();
            u8 unk[6];
            for (const auto i : algo::range(6))
                unk[i] = stream.read<u8>();
            out.comp->unk[0] = unk[0];
            out.comp->unk[1] = unk[1];
            out.comp->unk[2] = unk[4] >> 4;
            out.comp->unk[3] = unk[4] & 0x0F;
            out.comp->unk[4] = unk[2] + 1;
            out.comp->unk[5] = unk[2 + unk[5]] + 1;
            out.comp->unk[6] = out.comp->unk[4] - out.comp->unk[5];
            out.comp->unk[7] = 0;
        }

        else if (magic == "comp"_b)
        {
            out.comp = std::make_unique<CompChunk>();
            out.comp->block_size = stream.read_be<u16>();
            for (const auto i : algo::range(8))
                out.comp->unk[i] = stream.read<u8>();
            stream.skip(2);
        }

        else if (magic == "vbr\x00"_b)
        {
            out.vbr = std::make_unique<VbrChunk>();
            out.vbr->unk[0] = stream.read_be<u16>();
            out.vbr->unk[1] = stream.read_be<u16>();
        }

        else if (magic == "ath\x00"_b)
        {
            out.ath = std::make_unique<AthChunk>();
            out.ath->type = stream.read_le<u16>();
        }

        else if (magic == "loop"_b)
        {
            out.loop = std::make_unique<LoopChunk>();
            out.loop->start = stream.read_be<u32>();
            out.loop->end = stream.read_be<u32>();
            out.loop->repetitions = stream.read_be<u16>();
            stream.skip(2);
        }

        else if (magic == "ciph"_b)
        {
            out.ciph = std::make_unique<CiphChunk>();
            out.ciph->type = stream.read_be<u16>();
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
            const auto tmp = stream.read_be<u32>();
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
