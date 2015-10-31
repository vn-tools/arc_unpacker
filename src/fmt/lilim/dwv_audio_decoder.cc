#include "fmt/lilim/dwv_audio_decoder.h"
#include "util/file_from_samples.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic = "DW"_b;

bool DwvAudioDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("dwv") && file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> DwvAudioDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size() + 2);
    const auto header_size = file.io.read_u32_le();
    const auto samples_size = file.io.read_u32_le();
    const auto header = file.io.read(header_size);
    const auto samples = file.io.read(samples_size);

    auto output_file = std::make_unique<File>();
    output_file->io.write("RIFF"_b);
    output_file->io.write("\x00\x00\x00\x00"_b);
    output_file->io.write("WAVE"_b);
    output_file->io.write("fmt "_b);
    output_file->io.write_u32_le(header.size());
    output_file->io.write(header);
    output_file->io.write("data"_b);
    output_file->io.write_u32_le(samples.size());
    output_file->io.write(samples);
    output_file->io.seek(4);
    output_file->io.write_u32_le(output_file->io.size() - 8);

    output_file->name = file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::register_fmt<DwvAudioDecoder>("lilim/dwv");
