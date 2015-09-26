#include "fmt/touhou/pak1_audio_archive_decoder.h"
#include "util/audio.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static std::unique_ptr<File> read_audio(io::IO &arc_io, size_t index)
{
    auto size = arc_io.read_u32_le();
    auto format = arc_io.read_u16_le();
    auto channel_count = arc_io.read_u16_le();
    auto sample_rate = arc_io.read_u32_le();
    auto byte_rate = arc_io.read_u32_le();
    auto block_align = arc_io.read_u16_le();
    auto bits_per_sample = arc_io.read_u16_le();
    arc_io.skip(2);

    auto audio = util::Audio::from_samples(
        channel_count,
        bits_per_sample / 8,
        sample_rate,
        arc_io.read(size));

    return audio->create_file(util::format("%04d", index));
}

bool Pak1AudioArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    if (!arc_file.has_extension("dat"))
        return false;
    size_t file_count = arc_file.io.read_u32_le();
    for (auto i : util::range(file_count))
    {
        if (!arc_file.io.read_u8())
            continue;
        auto size = arc_file.io.read_u32_le();
        arc_file.io.skip(18);
        arc_file.io.skip(size);
    }
    return arc_file.io.eof();
}

void Pak1AudioArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &saver) const
{
    size_t file_count = arc_file.io.read_u32_le();
    for (auto i : util::range(file_count))
    {
        if (!arc_file.io.read_u8())
            continue;

        saver.save(read_audio(arc_file.io, i));
    }
}

static auto dummy = fmt::Registry::add<Pak1AudioArchiveDecoder>("th/pak1-sfx");
