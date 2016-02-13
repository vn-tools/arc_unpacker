#include "dec/alice_soft/dat_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "algo/str.h"
#include "err.h"

using namespace au;
using namespace au::dec::alice_soft;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    Logger dummy_logger;
    dummy_logger.mute();
    try
    {
        read_meta(dummy_logger, input_file);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<dec::ArchiveMeta> DatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto arc_name = algo::lower(input_file.path.stem());
    const auto header_size = (input_file.stream.read_le<u16>() - 1) * 256;
    auto meta = std::make_unique<ArchiveMeta>();

    ArchiveEntryImpl *last_entry = nullptr;
    bool finished = false;
    for (const auto i : algo::range((header_size / 2) - 1))
    {
        auto offset = input_file.stream.read_le<u16>();
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
        if (offset > input_file.stream.size())
            throw err::BadDataOffsetError();
        if (offset == input_file.stream.size())
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
        entry->path = algo::format("%03d.%s", i, ext.c_str());
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
        last_entry->size = input_file.stream.size() - last_entry->offset;
    else
        throw err::CorruptDataError("File table cannot be empty");

    return meta;
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> DatArchiveDecoder::get_linked_formats() const
{
    return {"alice-soft/vsp"};
}

static auto _ = dec::register_decoder<DatArchiveDecoder>("alice-soft/dat");
