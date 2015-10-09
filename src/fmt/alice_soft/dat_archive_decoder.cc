#include "fmt/alice_soft/dat_archive_decoder.h"
#include <boost/algorithm/string.hpp>
#include "err.h"
#include "fmt/alice_soft/vsp_image_decoder.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct DatArchiveDecoder::Priv final
{
    VspImageDecoder vsp_image_decoder;
};

DatArchiveDecoder::DatArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->vsp_image_decoder);
}

DatArchiveDecoder::~DatArchiveDecoder()
{
}

bool DatArchiveDecoder::is_recognized_impl(File &arc_file) const
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
    DatArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto arc_name = arc_file.name;
    boost::algorithm::to_lower(arc_name);

    auto header_size = (arc_file.io.read_u16_le() - 1) * 256;
    auto meta = std::make_unique<ArchiveMeta>();

    ArchiveEntryImpl *last_entry = nullptr;
    bool finished = false;
    for (auto i : util::range((header_size / 2) - 1))
    {
        size_t offset = arc_file.io.read_u16_le();
        if (offset == 0)
        {
            finished = true;
            continue;
        }
        offset = (offset - 1) * 256;

        if (last_entry && offset < last_entry->offset)
            throw err::CorruptDataError("Expected offsets to be sorted");
        if (finished)
            throw err::CorruptDataError("Expected remaining offsets to be 0");
        if (offset > arc_file.io.size())
            throw err::BadDataOffsetError();
        if (offset == arc_file.io.size())
            continue;

        // necessary for VSP recognition
        std::string ext = "dat";
        if (arc_name.find("cg") != std::string::npos)
            ext = "vsp";
        else if (arc_name.find("dis") != std::string::npos)
            ext = "sco";
        else if (arc_name.find("mus") != std::string::npos)
            ext = "mus";
        else if (arc_name.find("map") != std::string::npos)
            ext = "map";

        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = offset;
        entry->name = util::format("%03d.%s", i, ext.c_str());
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
        last_entry->size = arc_file.io.size() - last_entry->offset;
    else
        throw err::CorruptDataError("File table cannot be empty");

    return meta;
}

std::unique_ptr<File> DatArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<DatArchiveDecoder>("alice/dat");
