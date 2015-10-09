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
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> WpnAudioDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size());
    auto chunk_count = file.io.read_u32_le();

    std::vector<Chunk> chunks;
    for (auto i : util::range(chunk_count))
    {
        Chunk chunk;
        chunk.prefix = file.io.read(4);
        chunk.offset = file.io.read_u32_le();
        chunk.size = file.io.read_u32_le();
        chunks.push_back(chunk);
    }

    auto output_file = std::make_unique<File>();
    output_file->io.write("RIFF"_b);
    output_file->io.write_u32_le(0);
    output_file->io.write("WAVE"_b);

    for (auto &chunk : chunks)
    {
        output_file->io.write(chunk.prefix);
        output_file->io.write_u32_le(chunk.size);
        file.io.seek(chunk.offset);
        output_file->io.write_from_io(file.io, chunk.size);
    }

    output_file->io.seek(4);
    output_file->io.write_u32_le(output_file->io.size() - 8);
    output_file->name = file.name;
    output_file->change_extension("wav");
    return output_file;
}

static auto dummy = fmt::Registry::add<WpnAudioDecoder>("wild-bug/wpn");
