#include "util/file_from_samples.h"

using namespace au;
using namespace au::util;

std::unique_ptr<io::File> util::file_from_samples(
    size_t channel_count,
    size_t bits_per_sample,
    size_t sample_rate,
    const bstr &samples,
    const io::path &name)
{
    size_t block_align = ((channel_count * bits_per_sample) + 7) / 8;
    size_t byte_rate = block_align * sample_rate;
    size_t data_chunk_size = samples.size();

    auto output_file = std::make_unique<io::File>();

    output_file->stream.write("RIFF"_b);
    output_file->stream.write("\x00\x00\x00\x00"_b);
    output_file->stream.write("WAVE"_b);
    output_file->stream.write("fmt "_b);
    output_file->stream.write_u32_le(16);
    output_file->stream.write_u16_le(1);
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

    output_file->name = name;
    output_file->change_extension("wav");
    return output_file;
}
