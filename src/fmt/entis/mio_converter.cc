// MIO music/sound file
//
// Company:   Leshade Entis
// Engine:    Entis
// Extension: .mio
// Archives:  -
//
// Known games:
// - [Cuffs] [050805] Sakura Musubi

#include "err.h"
#include "fmt/entis/common/enums.h"
#include "fmt/entis/common/sections.h"
#include "fmt/entis/mio_converter.h"
#include "fmt/entis/sound/lossless.h"
#include "util/format.h"
#include "util/sound.h"

using namespace au;
using namespace au::fmt::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x01\x00\x03\x00\x00\x00\x00"_b;
static const bstr magic3 = "Music Interleaved and Orthogonal transformed"_b;

static sound::MioHeader read_header(
    io::IO &io, common::SectionReader &section_reader)
{
    auto header_section = section_reader.get_section("Header");
    io.seek(header_section.offset);
    common::SectionReader header_section_reader(io);
    header_section = header_section_reader.get_section("SoundInf");
    io.seek(header_section.offset);

    sound::MioHeader header;
    header.version = io.read_u32_le();
    header.transformation
        = static_cast<common::Transformation>(io.read_u32_le());
    header.architecture = static_cast<common::Architecture>(io.read_u32_le());

    header.channel_count   = io.read_u32_le();
    header.sample_rate     = io.read_u32_le();
    header.blockset_count  = io.read_u32_le();
    header.subband_degree  = io.read_u32_le();
    header.sample_count    = io.read_u32_le();
    header.lapped_degree   = io.read_u32_le();
    header.bits_per_sample = io.read_u32_le();
    return header;
}

static std::vector<sound::MioChunk> read_chunks(
    io::IO &io, common::SectionReader &section_reader)
{
    auto stream_section = section_reader.get_section("Stream");
    io.seek(stream_section.offset);
    common::SectionReader chunk_section_reader(io);
    std::vector<sound::MioChunk> chunks;
    for (auto &chunk_section : chunk_section_reader.get_sections("SoundStm"))
    {
        io.seek(chunk_section.offset);
        sound::MioChunk chunk;
        chunk.version = io.read_u8();
        chunk.initial = io.read_u8() > 0;
        io.skip(2);
        chunk.sample_count = io.read_u32_le();
        chunk.data = io.read(chunk_section.size - 8);
        chunks.push_back(chunk);
    }
    return chunks;
}

bool MioConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic1.size()) == magic1
        && file.io.read(magic2.size()) == magic2
        && file.io.read(magic3.size()) == magic3;
}

std::unique_ptr<File> MioConverter::decode_internal(File &file) const
{
    file.io.seek(0x40);

    common::SectionReader section_reader(file.io);
    auto header = read_header(file.io, section_reader);
    auto chunks = read_chunks(file.io, section_reader);

    std::unique_ptr<sound::SoundDecoderImpl> impl;
    if (header.transformation == common::Transformation::Lossless)
        impl.reset(new sound::LosslessSoundDecoder(header));
    else
    {
        throw err::NotSupportedError(util::format(
            "Transformation type %d not supported", header.transformation));
    }

    bstr samples;
    for (auto chunk : chunks)
        samples += impl->process_chunk(chunk);

    auto sound = util::Sound::from_samples(
        header.channel_count,
        header.bits_per_sample / 8,
        header.sample_rate,
        samples);
    return sound->create_file(file.name);
}

static auto dummy = fmt::Registry::add<MioConverter>("entis/mio");
