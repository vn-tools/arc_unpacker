#include "fmt/wild_bug/wpn_audio_decoder.h"
#include <map>
#include "util/range.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WBD\x1AWAV\x00"_b;
static const bstr fmt_magic = "fmt\x20"_b;
static const bstr data_magic = "data"_b;

namespace
{
    struct Chunk final
    {
        size_t offset;
        size_t size;
    };
}

bool WpnAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> WpnAudioDecoder::decode_impl(
    io::File &input_file) const
{
    const auto chunk_count = input_file.stream.seek(magic.size()).read_u32_le();
    std::map<bstr, Chunk> chunks;
    for (const auto i : util::range(chunk_count))
    {
        const auto chunk_name = input_file.stream.read(4);
        Chunk chunk;
        chunk.offset = input_file.stream.read_u32_le();
        chunk.size = input_file.stream.read_u32_le();
        chunks[chunk_name] = chunk;
    }

    sfx::Wave audio;

    const auto &fmt_chunk = chunks.at(fmt_magic);
    input_file.stream.seek(fmt_chunk.offset);
    audio.fmt.pcm_type = input_file.stream.read_u16_le();
    audio.fmt.channel_count = input_file.stream.read_u16_le();
    audio.fmt.sample_rate = input_file.stream.read_u32_le();
    const auto byte_rate = input_file.stream.read_u32_le();
    const auto block_align = input_file.stream.read_u16_le();
    audio.fmt.bits_per_sample = input_file.stream.read_u16_le();
    if (input_file.stream.tell() - fmt_chunk.offset < fmt_chunk.size)
    {
        const auto extra_data_size = input_file.stream.read_u16_le();
        audio.fmt.extra_data = input_file.stream.read(extra_data_size);
    }

    const auto &data_chunk = chunks.at(data_magic);
    input_file.stream.seek(data_chunk.offset);
    audio.data.samples = input_file.stream.read(data_chunk.size);

    return util::file_from_wave(audio, input_file.name);
}

static auto dummy = fmt::register_fmt<WpnAudioDecoder>("wild-bug/wpn");
