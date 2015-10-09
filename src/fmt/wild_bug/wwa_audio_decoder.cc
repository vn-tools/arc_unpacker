#include "fmt/wild_bug/wwa_audio_decoder.h"
#include "fmt/wild_bug/wpx/decoder.h"
#include "io/buffered_io.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WPX\x1AWAV\x00"_b;

bool WwaAudioDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> WwaAudioDecoder::decode_impl(File &file) const
{
    wpx::Decoder decoder(file.io);
    io::BufferedIO metadata_io(decoder.read_plain_section(0x20));

    auto format_type = metadata_io.read_u16_le();
    auto channel_count = metadata_io.read_u16_le();
    auto sample_rate = metadata_io.read_u32_le();
    auto byte_rate = metadata_io.read_u32_le();
    auto block_align = metadata_io.read_u16_le();
    auto bits_per_sample = metadata_io.read_u16_le();

    auto samples = decoder.read_compressed_section(0x21);

    auto bytes_per_sample = bits_per_sample >> 3;
    auto data_chunk_size = samples.size();

    auto output_file = std::make_unique<File>();
    output_file->io.write("RIFF"_b);
    output_file->io.write("\x00\x00\x00\x00"_b);
    output_file->io.write("WAVE"_b);
    output_file->io.write("fmt "_b);
    output_file->io.write_u32_le(16);
    output_file->io.write_u16_le(format_type);
    output_file->io.write_u16_le(channel_count);
    output_file->io.write_u32_le(sample_rate);
    output_file->io.write_u32_le(byte_rate);
    output_file->io.write_u16_le(block_align);
    output_file->io.write_u16_le(bits_per_sample);
    output_file->io.write("data"_b);
    output_file->io.write_u32_le(data_chunk_size);
    output_file->io.write(samples);
    output_file->io.seek(4);
    output_file->io.write_u32_le(output_file->io.size() - 8);

    output_file->name = file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::register_fmt<WwaAudioDecoder>("wild-bug/wwa");
