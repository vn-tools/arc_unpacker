// PAK1 sound file
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 07.5 - Immaterial and Missing Power

#include "fmt/touhou/pak1_sound_archive.h"
#include "util/itos.h"
#include "util/sound.h"

using namespace au;
using namespace au::fmt::touhou;

static std::unique_ptr<File> read_sound(io::IO &arc_io, size_t index)
{
    size_t size = arc_io.read_u32_le();
    auto format = arc_io.read_u16_le();
    auto channel_count = arc_io.read_u16_le();
    auto sample_rate = arc_io.read_u32_le();
    auto byte_rate = arc_io.read_u32_le();
    auto block_align = arc_io.read_u16_le();
    auto bits_per_sample = arc_io.read_u16_le();
    arc_io.skip(2);

    auto sound = util::Sound::from_samples(
        channel_count,
        bits_per_sample / 8,
        sample_rate,
        arc_io.read(size));

    return sound->create_file(util::itos(index, 4));
}

bool Pak1SoundArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.name.find("wave") != std::string::npos;
}

void Pak1SoundArchive::unpack_internal(
    File &arc_file, FileSaver &file_saver) const
{
    size_t file_count = arc_file.io.read_u32_le();
    for (size_t i = 0; i < file_count; i++)
    {
        if (!arc_file.io.read_u8())
            continue;

        file_saver.save(read_sound(arc_file.io, i));
    }
}
