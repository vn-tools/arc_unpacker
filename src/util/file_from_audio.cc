#include "util/file_from_audio.h"
#include "log.h"

using namespace au;
using namespace au::util;

std::unique_ptr<io::File> util::file_from_audio(
    const res::Audio &audio, const io::path &path)
{
    const auto block_align = audio.channel_count * audio.bits_per_sample / 8;
    const auto byte_rate = audio.sample_rate * block_align;

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("RIFF"_b);
    output_file->stream.write("\x00\x00\x00\x00"_b);
    output_file->stream.write("WAVE"_b);

    output_file->stream.write("fmt "_b);
    output_file->stream.write_u32_le(18 + audio.extra_codec_headers.size());
    output_file->stream.write_u16_le(audio.codec);
    output_file->stream.write_u16_le(audio.channel_count);
    output_file->stream.write_u32_le(audio.sample_rate);
    output_file->stream.write_u32_le(byte_rate);
    output_file->stream.write_u16_le(block_align);
    output_file->stream.write_u16_le(audio.bits_per_sample);
    output_file->stream.write_u16_le(audio.extra_codec_headers.size());
    output_file->stream.write(audio.extra_codec_headers);

    if (!audio.loops.empty())
        Log.warn("Audio loop found, but loops are not supported\n");

    output_file->stream.write("data"_b);
    output_file->stream.write_u32_le(audio.samples.size());
    output_file->stream.write(audio.samples);

    output_file->stream.seek(4);
    output_file->stream.write_u32_le(output_file->stream.size() - 8);

    output_file->path = path;
    output_file->path.change_extension("wav");
    return output_file;
}
