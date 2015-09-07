// Packed OGG sound file
//
// Company:   Liar-soft
// Engine:    -
// Extension: .wav
// Archives:  XFL
//
// Known games:
// - [Liar-soft] [060707] Souten No Celenaria - What a Beautiful World
// - [Liar-soft] [071122] Sekien no Inganock - What a Beautiful People
// - [Liar-soft] [081121] Shikkoku no Sharnoth - What a Beautiful Tomorrow

#include "fmt/liar_soft/packed_ogg_converter.h"
#include "io/buffered_io.h"
#include "util/range.h"
#include "util/require.h"

using namespace au;
using namespace au::fmt::liar_soft;

static const bstr ogg_magic = "OggS"_b;

bool PackedOggConverter::is_recognized_internal(File &file) const
{
    if (!file.has_extension("wav"))
        return false;
    file.io.seek(16);
    file.io.skip(file.io.read_u32_le());
    file.io.skip(20);
    return file.io.read(4) == ogg_magic;
}

static void rewrite_ogg_stream(io::IO &ogg_io, io::IO &target_io)
{
    // The OGG files used by LiarSoft may contain multiple streams, out of
    // which only the first one contains actual audio data.

    u32 initial_serial_number = 0;
    bool serial_number_known = false;
    while (!ogg_io.eof())
    {
        util::require(ogg_io.read(4) == ogg_magic);
        auto version = ogg_io.read_u8();
        auto header_type = ogg_io.read_u8();
        auto granule_position = ogg_io.read(8);
        auto bitstream_serial_number = ogg_io.read_u32_le();
        auto page_sequence_number = ogg_io.read_u32_le();
        auto checksum = ogg_io.read_u32_le();

        std::vector<bstr> page_segments(ogg_io.read_u8());
        for (auto &segment : page_segments)
            segment.resize(ogg_io.read_u8());

        for (auto &segment : page_segments)
            ogg_io.read(segment.get<u8>(), segment.size());

        if (!serial_number_known)
        {
            initial_serial_number = bitstream_serial_number;
            serial_number_known = true;
        }

        // The extra streams cause problems with popular (notably, all
        // ffmpeg-based) audio players, so we discard these streams here.
        if (bitstream_serial_number != initial_serial_number)
            continue;

        target_io.write(ogg_magic);
        target_io.write_u8(version);
        target_io.write_u8(header_type);
        target_io.write(granule_position);
        target_io.write_u32_le(bitstream_serial_number);
        target_io.write_u32_le(page_sequence_number);
        target_io.write_u32_le(checksum);
        target_io.write_u8(page_segments.size());
        for (auto &page_segment : page_segments)
            target_io.write_u8(page_segment.size());
        for (auto &page_segment : page_segments)
            target_io.write(page_segment);
    }
}

std::unique_ptr<File> PackedOggConverter::decode_internal(File &file) const
{
    util::require(file.io.read(4) == "RIFF"_b);
    file.io.skip(4); //file size without RIFF header - usually corrupted
    util::require(file.io.read(4) == "WAVE"_b);
    util::require(file.io.read(4) == "fmt\x20"_b);
    file.io.skip(file.io.read_u32_le());

    util::require(file.io.read(4) == "fact"_b);
    util::require(file.io.read_u32_le() == 4);
    file.io.skip(4);

    util::require(file.io.read(4) == "data"_b);
    auto data_size = file.io.read_u32_le();
    io::BufferedIO ogg_io(file.io, data_size);

    std::unique_ptr<File> output_file(new File);
    rewrite_ogg_stream(ogg_io, output_file->io);
    output_file->name = file.name;
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<PackedOggConverter>("liar/ogg");
