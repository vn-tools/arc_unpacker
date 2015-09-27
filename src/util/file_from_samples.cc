#include "util/file_from_samples.h"

using namespace au;
using namespace au::util;

std::unique_ptr<File> util::file_from_samples(
    size_t channel_count,
    size_t bytes_per_sample,
    size_t sample_rate,
    const bstr &samples,
    const std::string &name)
{
    size_t block_align = channel_count * bytes_per_sample;
    size_t byte_rate = block_align * sample_rate;
    size_t bits_per_sample = bytes_per_sample * 8;
    size_t data_chunk_size = samples.size();

    std::unique_ptr<File> output_file(new File);

    output_file->io.write("RIFF"_b);
    output_file->io.write("\x00\x00\x00\x00"_b);
    output_file->io.write("WAVE"_b);
    output_file->io.write("fmt "_b);
    output_file->io.write_u32_le(16);
    output_file->io.write_u16_le(1);
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

    output_file->name = name;
    output_file->change_extension("wav");
    return output_file;
}
