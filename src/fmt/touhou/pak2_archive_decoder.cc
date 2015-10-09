#include "fmt/touhou/pak2_archive_decoder.h"
#include <boost/filesystem.hpp>
#include "err.h"
#include "fmt/touhou/pak2_audio_decoder.h"
#include "fmt/touhou/pak2_image_decoder.h"
#include "io/buffered_io.h"
#include "io/file_io.h"
#include "util/encoding.h"
#include "util/mt.h"
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

static void decrypt(bstr &buffer, u32 mt_seed, u8 a, u8 b, u8 delta)
{
    util::mt::init_genrand(mt_seed);
    for (auto i : util::range(buffer.size()))
    {
        buffer[i] ^= util::mt::genrand_int32();
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

struct Pak2ArchiveDecoder::Priv final
{
    Pak2ImageDecoder image_decoder;
    Pak2AudioDecoder audio_decoder;
};

Pak2ArchiveDecoder::Pak2ArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->image_decoder);
    add_decoder(&p->audio_decoder);
}

Pak2ArchiveDecoder::~Pak2ArchiveDecoder()
{
}

bool Pak2ArchiveDecoder::is_recognized_impl(File &arc_file) const
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
    Pak2ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(0);
    u16 file_count = arc_file.io.read_u16_le();
    if (file_count == 0 && arc_file.io.size() != 6)
        throw err::RecognitionError();

    size_t table_size = arc_file.io.read_u32_le();
    if (table_size > arc_file.io.size() - arc_file.io.tell())
        throw err::RecognitionError();
    if (table_size > file_count * (4 + 4 + 256 + 1))
        throw err::RecognitionError();
    auto table_data = arc_file.io.read(table_size);
    decrypt(table_data, table_size + 6, 0xC5, 0x83, 0x53);
    io::BufferedIO table_io(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = table_io.read_u32_le();
        entry->size = table_io.read_u32_le();
        auto name_size = table_io.read_u8();
        entry->name = util::sjis_to_utf8(table_io.read(name_size)).str();
        if (entry->offset + entry->size > arc_file.io.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> Pak2ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    u8 key = (entry->offset >> 1) | 0x23;
    for (auto i : util::range(entry->size))
        data[i] ^= key;
    return std::make_unique<File>(entry->name, data);
}

void Pak2ArchiveDecoder::preprocess(
    File &arc_file, ArchiveMeta &m, const FileSaver &) const
{
    p->image_decoder.clear_palettes();
    auto dir = boost::filesystem::path(arc_file.name).parent_path();
    for (boost::filesystem::directory_iterator it(dir);
        it != boost::filesystem::directory_iterator();
        it++)
    {
        if (!boost::filesystem::is_regular_file(it->path()))
            continue;
        if (it->path().string().find(".dat") == std::string::npos)
            continue;

        File other_arc_file(it->path().string(), io::FileMode::Read);
        if (!is_recognized(other_arc_file))
            continue;

        auto meta = read_meta(other_arc_file);
        for (auto &entry : meta->entries)
        {
            if (entry->name.find(".pal") == std::string::npos)
                continue;

            auto pal_file = read_file(other_arc_file, *meta, *entry);
            pal_file->io.seek(0);
            p->image_decoder.add_palette(
                entry->name, pal_file->io.read_to_eof());
        }
    }
}

static auto dummy = fmt::Registry::add<Pak2ArchiveDecoder>("th/pak2");
