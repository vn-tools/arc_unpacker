#include "fmt/wild_bug/wwa_audio_decoder.h"
#include "fmt/wild_bug/wpx/decoder.h"
#include "io/memory_stream.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WPX\x1AWAV\x00"_b;

bool WwaAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> WwaAudioDecoder::decode_impl(
    io::File &input_file) const
{
    wpx::Decoder decoder(input_file.stream);
    io::MemoryStream metadata_stream(decoder.read_plain_section(0x20));

    const auto format_type = metadata_stream.read_u16_le();
    const auto channel_count = metadata_stream.read_u16_le();
    const auto sample_rate = metadata_stream.read_u32_le();
    const auto byte_rate = metadata_stream.read_u32_le();
    const auto block_align = metadata_stream.read_u16_le();
    const auto bits_per_sample = metadata_stream.read_u16_le();

    const auto samples = decoder.read_compressed_section(0x21);

    sfx::Wave audio;
    audio.fmt.pcm_type = format_type;
    audio.fmt.channel_count = channel_count;
    audio.fmt.sample_rate = sample_rate;
    audio.fmt.bits_per_sample = bits_per_sample;
    audio.data.samples = samples;
    return util::file_from_wave(audio, input_file.name);
}

static auto dummy = fmt::register_fmt<WwaAudioDecoder>("wild-bug/wwa");
