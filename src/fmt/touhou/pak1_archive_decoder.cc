#include "fmt/touhou/pak1_archive_decoder.h"
#include "err.h"
#include "fmt/touhou/pak1_audio_archive_decoder.h"
#include "fmt/touhou/pak1_image_archive_decoder.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static void decrypt(bstr &buffer, u8 a, u8 b, u8 delta)
{
    for (auto i : util::range(buffer.size()))
    {
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

static std::unique_ptr<io::BufferedIO> read_raw_table(
    io::IO &arc_io, size_t file_count)
{
    size_t table_size = file_count * 0x6C;
    if (table_size > arc_io.size() - arc_io.tell())
        throw err::RecognitionError();
    if (table_size > file_count * (0x64 + 4 + 4))
        throw err::RecognitionError();
    auto buffer = arc_io.read(table_size);
    decrypt(buffer, 0x64, 0x64, 0x4D);
    return std::make_unique<io::BufferedIO>(buffer);
}

struct Pak1ArchiveDecoder::Priv final
{
    Pak1ImageArchiveDecoder image_archive_decoder;
    Pak1AudioArchiveDecoder audio_archive_decoder;
};

Pak1ArchiveDecoder::Pak1ArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->image_archive_decoder);
    add_decoder(&p->audio_archive_decoder);
}

Pak1ArchiveDecoder::~Pak1ArchiveDecoder()
{
}

bool Pak1ArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    try
    {
        read_meta(arc_file);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<fmt::ArchiveMeta>
    Pak1ArchiveDecoder::read_meta(File &arc_file) const
{
    u16 file_count = arc_file.io.read_u16_le();
    if (file_count == 0 && arc_file.io.size() != 6)
        throw err::RecognitionError();
    auto table_io = read_raw_table(arc_file.io, file_count);
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_io->read_to_zero(0x64).str();
        entry->size = table_io->read_u32_le();
        entry->offset = table_io->read_u32_le();
        if (entry->offset + entry->size > arc_file.io.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> Pak1ArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto output_file = std::make_unique<File>();
    output_file->name = entry->name;

    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);

    if (output_file->name.find("musicroom.dat") != std::string::npos)
    {
        decrypt(data, 0x5C, 0x5A, 0x3D);
        output_file->change_extension(".txt");
    }
    else if (output_file->name.find(".sce") != std::string::npos)
    {
        decrypt(data, 0x63, 0x62, 0x42);
        output_file->change_extension(".txt");
    }
    else if (output_file->name.find("cardlist.dat") != std::string::npos)
    {
        decrypt(data, 0x60, 0x61, 0x41);
        output_file->change_extension(".txt");
    }

    output_file->io.write(data);
    return output_file;
}

static auto dummy = fmt::Registry::add<Pak1ArchiveDecoder>("th/pak1");
