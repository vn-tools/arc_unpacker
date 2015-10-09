#include "fmt/entis/noa_archive_decoder.h"
#include "fmt/entis/common/sections.h"
#include "fmt/entis/eri_image_decoder.h"
#include "fmt/entis/mio_audio_decoder.h"
#include "log.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x04\x00\x02\x00\x00\x00\x00"_b;
static const bstr magic3 = "ERISA-Archive file"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool encrypted;
        bstr extra;
    };
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta(
    io::IO &io, std::string root = "")
{
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    common::SectionReader section_reader(io);
    for (auto &section : section_reader.get_sections("DirEntry"))
    {
        io.seek(section.offset);
        auto entry_count = io.read_u32_le();
        for (auto i : util::range(entry_count))
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            entry->size = io.read_u64_le();
            auto flags = io.read_u32_le();
            entry->encrypted = io.read_u32_le() > 0;
            entry->offset = section.offset + io.read_u64_le();
            io.skip(8);

            auto extra_size = io.read_u32_le();
            if (flags & 0x70)
                entry->extra = io.read(extra_size);

            entry->name = io.read_to_zero(io.read_u32_le()).str();
            if (root != "")
                entry->name = root + "/" + entry->name;

            if (flags == 0x10)
            {
                io.peek(entry->offset, [&]()
                {
                    for (auto &sub_entry : read_meta(io, entry->name)->entries)
                        meta->entries.push_back(std::move(sub_entry));
                });
            }
            else if (flags == 0x20 || flags == 0x40)
            {
            }
            else
            {
                meta->entries.push_back(std::move(entry));
            }
        }
    }
    return meta;
}

struct NoaArchiveDecoder::Priv final
{
    EriImageDecoder eri_image_decoder;
    MioAudioDecoder mio_audio_decoder;
};

NoaArchiveDecoder::NoaArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->eri_image_decoder);
    add_decoder(&p->mio_audio_decoder);
}

NoaArchiveDecoder::~NoaArchiveDecoder()
{
}

bool NoaArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic1.size()) == magic1
        && arc_file.io.read(magic2.size()) == magic2
        && arc_file.io.read(magic3.size()) == magic3;
}

std::unique_ptr<fmt::ArchiveMeta>
    NoaArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(0x40);
    return ::read_meta(arc_file.io);
}

std::unique_ptr<File> NoaArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->encrypted)
    {
        Log.warn(util::format(
            "%s is encrypted, but encrypted files are not supported\n",
            entry->name.c_str()));
    }
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<NoaArchiveDecoder>("entis/noa");
