#include "util/file_from_wave.h"
#include "util/range.h"

using namespace au;
using namespace au::util;

std::unique_ptr<io::File> util::file_from_wave(
    const sfx::Wave &audio, const std::string &name)
{
    const auto block_align
        = audio.fmt.channel_count * audio.fmt.bits_per_sample / 8;
    const auto byte_rate = audio.fmt.sample_rate * block_align;

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("RIFF"_b);
    output_file->stream.write("\x00\x00\x00\x00"_b);
    output_file->stream.write("WAVE"_b);

    output_file->stream.write("fmt "_b);
    output_file->stream.write_u32_le(18 + audio.fmt.extra_data.size());
    output_file->stream.write_u16_le(audio.fmt.pcm_type);
    output_file->stream.write_u16_le(audio.fmt.channel_count);
    output_file->stream.write_u32_le(audio.fmt.sample_rate);
    output_file->stream.write_u32_le(byte_rate);
    output_file->stream.write_u16_le(block_align);
    output_file->stream.write_u16_le(audio.fmt.bits_per_sample);
    output_file->stream.write_u16_le(audio.fmt.extra_data.size());
    output_file->stream.write(audio.fmt.extra_data);

    if (audio.smpl)
    {
        output_file->stream.write("smpl"_b);
        output_file->stream.write_u32_le(36
            + (24 * audio.smpl->loops.size())
            + audio.smpl->extra_data.size());
        output_file->stream.write_u32_le(audio.smpl->manufacturer);
        output_file->stream.write_u32_le(audio.smpl->product);
        output_file->stream.write_u32_le(audio.smpl->sample_period);
        output_file->stream.write_u32_le(audio.smpl->midi_unity_note);
        output_file->stream.write_u32_le(audio.smpl->midi_pitch_fraction);
        output_file->stream.write_u32_le(audio.smpl->smpte_format);
        output_file->stream.write_u32_le(audio.smpl->smpte_offset);
        output_file->stream.write_u32_le(audio.smpl->loops.size());
        output_file->stream.write_u32_le(audio.smpl->extra_data.size());
        for (const auto i : util::range(audio.smpl->loops.size()))
        {
            const auto loop = audio.smpl->loops[i];
            output_file->stream.write_u32_le(i);
            output_file->stream.write_u32_le(loop.type);
            output_file->stream.write_u32_le(loop.start);
            output_file->stream.write_u32_le(loop.end);
            output_file->stream.write_u32_le(loop.fraction);
            output_file->stream.write_u32_le(loop.play_count);
        }
        output_file->stream.write(audio.smpl->extra_data);
    }

    output_file->stream.write("data"_b);
    output_file->stream.write_u32_le(audio.data.samples.size());
    output_file->stream.write(audio.data.samples);

    output_file->stream.seek(4);
    output_file->stream.write_u32_le(output_file->stream.size() - 8);

    output_file->name = name;
    output_file->change_extension("wav");
    return output_file;
}
