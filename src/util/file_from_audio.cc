#include "util/file_from_audio.h"
#include "algo/range.h"

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

    output_file->stream.write("data"_b);
    output_file->stream.write_u32_le(audio.samples.size());
    output_file->stream.write(audio.samples);

    if (!audio.loops.empty())
    {
        const auto extra_data = ""_b;
        output_file->stream.write("smpl"_b);
        output_file->stream.write_u32_le(36
            + (24 * audio.loops.size()) + extra_data.size());
        output_file->stream.write_u32_le(0); // manufacturer
        output_file->stream.write_u32_le(0); // product
        output_file->stream.write_u32_le(0); // sample period
        output_file->stream.write_u32_le(0); // midi unity note
        output_file->stream.write_u32_le(0); // midi pitch fraction
        output_file->stream.write_u32_le(0); // smpte format
        output_file->stream.write_u32_le(0); // smpte offset
        output_file->stream.write_u32_le(audio.loops.size());
        output_file->stream.write_u32_le(extra_data.size());
        for (const auto i : algo::range(audio.loops.size()))
        {
            const auto loop = audio.loops[i];
            output_file->stream.write_u32_le(i);
            output_file->stream.write_u32_le(0); // type
            output_file->stream.write_u32_le(loop.start);
            output_file->stream.write_u32_le(loop.end);
            output_file->stream.write_u32_le(0); // fraction
            output_file->stream.write_u32_le(loop.play_count);
        }
        output_file->stream.write(extra_data);
    }

    output_file->stream.seek(4);
    output_file->stream.write_u32_le(output_file->stream.size() - 8);

    output_file->path = path;
    if (!audio.loops.empty())
        output_file->path.change_extension("wavloop");
    else
        output_file->path.change_extension("wav");
    return output_file;
}
