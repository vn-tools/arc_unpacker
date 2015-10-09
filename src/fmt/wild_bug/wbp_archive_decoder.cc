#include "fmt/wild_bug/wbp_archive_decoder.h"
#include <map>
#include "fmt/wild_bug/wbi_file_decoder.h"
#include "fmt/wild_bug/wbm_image_decoder.h"
#include "fmt/wild_bug/wpn_audio_decoder.h"
#include "fmt/wild_bug/wwa_audio_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "ARCFORM4\x20WBUG\x20"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct WbpArchiveDecoder::Priv final
{
    WpnAudioDecoder wpn_audio_decoder;
    WbmImageDecoder wbm_image_decoder;
    WwaAudioDecoder wwa_audio_decoder;
    WbiFileDecoder wbi_file_decoder;
};

WbpArchiveDecoder::WbpArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->wpn_audio_decoder);
    add_decoder(&p->wbm_image_decoder);
    add_decoder(&p->wwa_audio_decoder);
    add_decoder(&p->wbi_file_decoder);
}

WbpArchiveDecoder::~WbpArchiveDecoder()
{
}

bool WbpArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    WbpArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(0x10);
    auto file_count = arc_file.io.read_u32_le();
    auto table_offset = arc_file.io.read_u32_le();
    auto table_size = arc_file.io.read_u32_le();
    arc_file.io.skip(8);

    std::vector<size_t> dir_offsets;
    for (auto i : util::range(0x100))
    {
        auto offset = arc_file.io.read_u32_le();
        if (offset)
            dir_offsets.push_back(offset);
    }

    std::vector<size_t> file_offsets;
    for (auto i : util::range(0x100))
    {
        auto offset = arc_file.io.read_u32_le();
        if (offset)
            file_offsets.push_back(offset);
    }

    std::map<u16, std::string> dir_names;
    for (auto &offset : dir_offsets)
    {
        arc_file.io.seek(offset + 1);
        auto name_size = arc_file.io.read_u8();
        auto dir_id = arc_file.io.read_u8();
        arc_file.io.skip(1);
        dir_names[dir_id] = arc_file.io.read_to_zero(name_size).str();
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (size_t i : util::range(file_offsets.size()))
    {
        // one file offset may contain multiple entries
        arc_file.io.seek(file_offsets[i]);
        do
        {
            auto old_pos = arc_file.io.tell();

            arc_file.io.skip(1);
            auto name_size = arc_file.io.read_u8();
            auto dir_id = arc_file.io.read_u8();
            arc_file.io.skip(1);

            auto entry = std::make_unique<ArchiveEntryImpl>();
            entry->offset = arc_file.io.read_u32_le();
            entry->size = arc_file.io.read_u32_le();
            arc_file.io.skip(8);
            entry->name = dir_names.at(dir_id)
                + arc_file.io.read_to_zero(name_size).str();

            meta->entries.push_back(std::move(entry));

            arc_file.io.seek(old_pos + (name_size & 0xFC) + 0x18);
        }
        while (i + 1 < file_offsets.size()
            && arc_file.io.tell() < file_offsets[i + 1]);
    }
    return meta;
}

std::unique_ptr<File> WbpArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<WbpArchiveDecoder>("wild-bug/wbp");
