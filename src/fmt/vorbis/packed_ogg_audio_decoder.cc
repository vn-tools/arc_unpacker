#include "fmt/vorbis/packed_ogg_audio_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"
#include "log.h"

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

static OggPage read_ogg_page(io::Stream &input)
{
    OggPage page;
    page.version = input.read_u8();
    page.header_type = input.read_u8();
    page.granule_position = input.read(8);
    page.bitstream_serial_number = input.read_u32_le();
    page.page_sequence_number = input.read_u32_le();
    page.checksum = input.read_u32_le();
    std::vector<size_t> sizes(input.read_u8());
    for (auto &size : sizes)
        size = input.read_u8();
    for (auto &size : sizes)
        page.segments.push_back(input.read(size));
    return page;
}

static void write_ogg_page(io::Stream &output, const OggPage &page)
{
    output.write(ogg_magic);
    output.write_u8(page.version);
    output.write_u8(page.header_type);
    output.write(page.granule_position);
    output.write_u32_le(page.bitstream_serial_number);
    output.write_u32_le(page.page_sequence_number);
    output.write_u32_le(page.checksum);
    output.write_u8(page.segments.size());
    for (auto &page_segment : page.segments)
        output.write_u8(page_segment.size());
    for (auto &page_segment : page.segments)
        output.write(page_segment);
}

bool PackedOggAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("wav"))
        return false;
    input_file.stream.seek(16);
    input_file.stream.skip(input_file.stream.read_u32_le());
    input_file.stream.skip(20);
    return input_file.stream.read(4) == ogg_magic;
}

static void rewrite_ogg_stream(io::Stream &input, io::Stream &output)
{
    // The OGG files used by LiarSoft may contain multiple streams, out of
    // which only the first one contains actual audio data.

    u32 initial_serial_number = 0;
    auto pages = 0;
    auto serial_number_known = false;
    while (!input.eof())
    {
        OggPage page;
        try
        {
            if (input.read(4) != ogg_magic)
                throw err::CorruptDataError("Expected OGG signature");

            page = read_ogg_page(input);
        }
        catch (err::IoError)
        {
            Log.warn(algo::format(
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
            write_ogg_page(output, page);
            pages++;
        }
    }
}

std::unique_ptr<io::File> PackedOggAudioDecoder::decode_impl(
    io::File &input_file) const
{
    if (input_file.stream.read(4) != "RIFF"_b)
        throw err::CorruptDataError("Expected RIFF signature");
    input_file.stream.skip(4); // file size w/o RIFF header, usually corrupted
    if (input_file.stream.read(8) != "WAVEfmt\x20"_b)
        throw err::CorruptDataError("Expected WAVE header");
    input_file.stream.skip(input_file.stream.read_u32_le());

    if (input_file.stream.read(4) != "fact"_b)
        throw err::CorruptDataError("Expected fact chunk");
    if (input_file.stream.read_u32_le() != 4)
        throw err::CorruptDataError("Corrupt fact chunk");
    input_file.stream.skip(4);

    if (input_file.stream.read(4) != "data"_b)
        throw err::CorruptDataError("Expected data chunk");
    auto data_size = input_file.stream.read_u32_le();
    io::MemoryStream input(input_file.stream, data_size);

    auto output_file = std::make_unique<io::File>();
    rewrite_ogg_stream(input, output_file->stream);
    output_file->path = input_file.path;
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<PackedOggAudioDecoder>("vorbis/wav");
