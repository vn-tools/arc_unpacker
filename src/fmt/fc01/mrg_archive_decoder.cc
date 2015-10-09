#include "fmt/fc01/mrg_archive_decoder.h"
#include "err.h"
#include "fmt/fc01/acd_image_decoder.h"
#include "fmt/fc01/common/custom_lzss.h"
#include "fmt/fc01/common/mrg_decryptor.h"
#include "fmt/fc01/common/util.h"
#include "fmt/fc01/mca_archive_decoder.h"
#include "fmt/fc01/mcg_image_decoder.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

static const bstr magic = "MRG\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        u32 offset;
        u32 size_orig;
        u32 size_comp;
        u8 filter;
    };
}

static u8 guess_key(const bstr &table_data, size_t file_size)
{
    u8 tmp = common::rol8(table_data.get<u8>()[table_data.size() - 1], 1);
    u8 key = tmp ^ (file_size >> 24);
    u32 pos = 1;
    u32 last_offset = tmp ^ key;
    for (auto i = table_data.size() - 2; i >= table_data.size() - 4; --i)
    {
        key -= ++pos;
        tmp = common::rol8(table_data.get<u8>()[i], 1);
        last_offset = (last_offset << 8) | (tmp ^ key);
    }
    if (last_offset != file_size)
        throw err::NotSupportedError("Failed to guess the key");
    while (pos++ < table_data.size())
        key -= pos;
    return key;
}

struct MrgArchiveDecoder::Priv final
{
    AcdImageDecoder acd_image_decoder;
    McaArchiveDecoder mca_archive_decoder;
    McgImageDecoder mcg_image_decoder;
};

MrgArchiveDecoder::MrgArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->acd_image_decoder);
    add_decoder(&p->mca_archive_decoder);
    add_decoder(&p->mcg_image_decoder);
}

MrgArchiveDecoder::~MrgArchiveDecoder()
{
}

bool MrgArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    MrgArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size() + 4);
    auto table_size = arc_file.io.read_u32_le() - 12 - magic.size();
    auto file_count = arc_file.io.read_u32_le();

    auto table_data = arc_file.io.read(table_size);
    auto key = guess_key(table_data, arc_file.io.size());
    for (auto i : util::range(table_data.size()))
    {
        table_data[i] = common::rol8(table_data[i], 1) ^ key;
        key += table_data.size() - i;
    }

    io::BufferedIO table_io(table_data);
    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_io.read_to_zero(0x0E).str();
        entry->size_orig = table_io.read_u32_le();
        entry->filter = table_io.read_u8();
        table_io.skip(9);
        entry->offset = table_io.read_u32_le();
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
    {
        table_io.skip(0x1C);
        last_entry->size_comp = table_io.read_u32_le() - last_entry->offset;
    }

    return meta;
}

std::unique_ptr<File> MrgArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp);
    if (entry->filter)
    {
        if (entry->filter >= 2)
        {
            common::MrgDecryptor decryptor(data);
            data = decryptor.decrypt(0);
        }
        if (entry->filter < 3)
            data = common::custom_lzss_decompress(data, entry->size_orig);
    }
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<MrgArchiveDecoder>("fc01/mrg");
