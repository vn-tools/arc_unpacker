#include "fmt/wild_bug/wpn_audio_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WBD\x1AWAV\x00"_b;

namespace
{
    struct Chunk final
    {
        bstr prefix;
        size_t offset;
        size_t size;
    };
}

bool WpnAudioDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> WpnAudioDecoder::decode_impl(File &file) const
{
    file.stream.seek(magic.size());
    auto chunk_count = file.stream.read_u32_le();

    std::vector<Chunk> chunks;
    for (auto i : util::range(chunk_count))
    {
        Chunk chunk;
        chunk.prefix = file.stream.read(4);
        chunk.offset = file.stream.read_u32_le();
        chunk.size = file.stream.read_u32_le();
        chunks.push_back(chunk);
    }

    auto output_file = std::make_unique<File>();
    output_file->stream.write("RIFF"_b);
    output_file->stream.write_u32_le(0);
    output_file->stream.write("WAVE"_b);

    for (auto &chunk : chunks)
    {
        output_file->stream.write(chunk.prefix);
        output_file->stream.write_u32_le(chunk.size);
        file.stream.seek(chunk.offset);
        output_file->stream.write(file.stream.read(chunk.size));
    }

    output_file->stream.seek(4);
    output_file->stream.write_u32_le(output_file->stream.size() - 8);
    output_file->name = file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::register_fmt<WpnAudioDecoder>("wild-bug/wpn");
