#include "fmt/wild_bug/wwa_audio_decoder.h"
#include "fmt/wild_bug/wpx/decoder.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WPX\x1AWAV\x00"_b;

bool WwaAudioDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> WwaAudioDecoder::decode_impl(File &input_file) const
{
    wpx::Decoder decoder(input_file.stream);
    io::MemoryStream metadata_stream(decoder.read_plain_section(0x20));

    auto format_type = metadata_stream.read_u16_le();
    auto channel_count = metadata_stream.read_u16_le();
    auto sample_rate = metadata_stream.read_u32_le();
    auto byte_rate = metadata_stream.read_u32_le();
    auto block_align = metadata_stream.read_u16_le();
    auto bits_per_sample = metadata_stream.read_u16_le();

    auto samples = decoder.read_compressed_section(0x21);

    auto bytes_per_sample = bits_per_sample >> 3;
    auto data_chunk_size = samples.size();

    auto output_file = std::make_unique<File>();
    output_file->stream.write("RIFF"_b);
    output_file->stream.write("\x00\x00\x00\x00"_b);
    output_file->stream.write("WAVE"_b);
    output_file->stream.write("fmt "_b);
    output_file->stream.write_u32_le(16);
    output_file->stream.write_u16_le(format_type);
    output_file->stream.write_u16_le(channel_count);
    output_file->stream.write_u32_le(sample_rate);
    output_file->stream.write_u32_le(byte_rate);
    output_file->stream.write_u16_le(block_align);
    output_file->stream.write_u16_le(bits_per_sample);
    output_file->stream.write("data"_b);
    output_file->stream.write_u32_le(data_chunk_size);
    output_file->stream.write(samples);
    output_file->stream.seek(4);
    output_file->stream.write_u32_le(output_file->stream.size() - 8);

    output_file->name = input_file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::register_fmt<WwaAudioDecoder>("wild-bug/wwa");
