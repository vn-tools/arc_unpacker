#include "fmt/vorbis/packed_ogg_converter.h"
#include "err.h"
#include "io/buffered_io.h"
#include "log.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::vorbis;

namespace
{
    struct OggPage final
    {
        u8 version;
        u8 header_type;
        bstr granule_position;
        u32 bitstream_serial_number;
        u32 page_sequence_number;
        u32 checksum;
        std::vector<bstr> segments;
    };
}

static const bstr ogg_magic = "OggS"_b;

static OggPage read_ogg_page(io::IO &io)
{
    OggPage page;
    page.version = io.read_u8();
    page.header_type = io.read_u8();
    page.granule_position = io.read(8);
    page.bitstream_serial_number = io.read_u32_le();
    page.page_sequence_number = io.read_u32_le();
    page.checksum = io.read_u32_le();
    page.segments.resize(io.read_u8());
    for (auto &segment : page.segments)
        segment.resize(io.read_u8());
    for (auto &segment : page.segments)
        io.read(segment.get<u8>(), segment.size());
    return page;
}

static void write_ogg_page(io::IO &target_io, const OggPage &page)
{
    target_io.write(ogg_magic);
    target_io.write_u8(page.version);
    target_io.write_u8(page.header_type);
    target_io.write(page.granule_position);
    target_io.write_u32_le(page.bitstream_serial_number);
    target_io.write_u32_le(page.page_sequence_number);
    target_io.write_u32_le(page.checksum);
    target_io.write_u8(page.segments.size());
    for (auto &page_segment : page.segments)
        target_io.write_u8(page_segment.size());
    for (auto &page_segment : page.segments)
        target_io.write(page_segment);
}

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
    auto pages = 0;
    auto serial_number_known = false;
    while (!ogg_io.eof())
    {
        OggPage page;
        try
        {
            if (ogg_io.read(4) != ogg_magic)
                throw err::CorruptDataError("Expected OGG signature");

            page = read_ogg_page(ogg_io);
        }
        catch (err::IoError)
        {
            Log.warn(util::format(
                "Last OGG page is truncated; recovered %d pages.\n", pages));
            break;
        }

        if (!serial_number_known)
        {
            initial_serial_number = page.bitstream_serial_number;
            serial_number_known = true;
        }

        // The extra streams cause problems with popular (notably, all
        // ffmpeg-based) audio players, so we discard these streams here.
        if (page.bitstream_serial_number == initial_serial_number)
        {
            write_ogg_page(target_io, page);
            pages++;
        }
    }
}

std::unique_ptr<File> PackedOggConverter::decode_internal(File &file) const
{
    if (file.io.read(4) != "RIFF"_b)
        throw err::CorruptDataError("Expected RIFF signature");
    file.io.skip(4); //file size without RIFF header - usually corrupted
    if (file.io.read(8) != "WAVEfmt\x20"_b)
        throw err::CorruptDataError("Expected WAVE header");
    file.io.skip(file.io.read_u32_le());

    if (file.io.read(4) != "fact"_b)
        throw err::CorruptDataError("Expected fact chunk");
    if (file.io.read_u32_le() != 4)
        throw err::CorruptDataError("Corrupt fact chunk");
    file.io.skip(4);

    if (file.io.read(4) != "data"_b)
        throw err::CorruptDataError("Expected data chunk");
    auto data_size = file.io.read_u32_le();
    io::BufferedIO ogg_io(file.io, data_size);

    std::unique_ptr<File> output_file(new File);
    rewrite_ogg_stream(ogg_io, output_file->io);
    output_file->name = file.name;
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<PackedOggConverter>("vorbis/wav");
