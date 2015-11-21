#include "fmt/kid/waf_audio_decoder.h"

using namespace au;
using namespace au::fmt::kid;

static const bstr magic = "WAF\x00\x00\x00"_b;

bool WafAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> WafAudioDecoder::decode_impl(
    io::File &input_file) const
{
    auto output_file = std::make_unique<io::File>();

    input_file.stream.skip(6);

    output_file->stream.write("RIFF"_b);
    output_file->stream.write("\x00\x00\x00\x00"_b);
    output_file->stream.write("WAVE"_b);
    output_file->stream.write("fmt\x20"_b);
    output_file->stream.write_u32_le(50); // fmt header size
    output_file->stream.write_u16_le(2); // compression type - ADPCM
    output_file->stream.write_u16_le(input_file.stream.read_u16_le());
    output_file->stream.write_u32_le(input_file.stream.read_u32_le());
    output_file->stream.write_u32_le(input_file.stream.read_u32_le());
    output_file->stream.write_u16_le(input_file.stream.read_u16_le());
    output_file->stream.write_u16_le(input_file.stream.read_u16_le());

    output_file->stream.write_u16_le(32); // size of additional header
    output_file->stream.write(input_file.stream.read(32)); // aditional header

    output_file->stream.write("data"_b);
    output_file->stream.write(input_file.stream.read_to_eof());

    output_file->stream.seek(4);
    output_file->stream.write_u32_le(input_file.stream.size());

    output_file->name = input_file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::register_fmt<WafAudioDecoder>("kid/waf");
