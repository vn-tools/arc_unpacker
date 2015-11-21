#include "fmt/lilim/dwv_audio_decoder.h"
#include "util/file_from_samples.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic = "DW"_b;

bool DwvAudioDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("dwv") && file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> DwvAudioDecoder::decode_impl(File &file) const
{
    file.stream.seek(magic.size() + 2);
    const auto header_size = file.stream.read_u32_le();
    const auto samples_size = file.stream.read_u32_le();
    const auto header = file.stream.read(header_size);
    const auto samples = file.stream.read(samples_size);

    auto output_file = std::make_unique<File>();
    output_file->stream.write("RIFF"_b);
    output_file->stream.write("\x00\x00\x00\x00"_b);
    output_file->stream.write("WAVE"_b);
    output_file->stream.write("fmt "_b);
    output_file->stream.write_u32_le(header.size());
    output_file->stream.write(header);
    output_file->stream.write("data"_b);
    output_file->stream.write_u32_le(samples.size());
    output_file->stream.write(samples);
    output_file->stream.seek(4);
    output_file->stream.write_u32_le(output_file->stream.size() - 8);

    output_file->name = file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::register_fmt<DwvAudioDecoder>("lilim/dwv");
