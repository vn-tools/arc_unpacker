#include "fmt/propeller/mgr_archive_decoder.h"
#include "io/memory_stream.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::propeller;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
    };
}

static bstr decompress(const bstr &input, size_t size_original)
{
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input_ptr + input.size();

    bstr output(size_original);
    u8 *output_ptr = output.get<u8>();
    u8 *output_end = output_ptr + size_original;

    while (output_ptr < output_end)
    {
        u32 c = *input_ptr++;

        if (c < 0x20)
        {
            u32 size = c + 1;
            while (size--)
                *output_ptr++ = *input_ptr++;
        }
        else
        {
            u32 size = c >> 5;
            if (size == 7)
                size += *input_ptr++;
            size += 2;

            u32 look_behind = ((c & 0x1F) << 8) + 1;
            look_behind += *input_ptr++;

            u8 *source = output_ptr - look_behind;
            while (size--)
                *output_ptr++ = *source++;
        }
    }

    return output;
}

bool MgrArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.has_extension("mgr");
}

std::unique_ptr<fmt::ArchiveMeta>
    MgrArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto entry_count = arc_file.stream.read_u16_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(entry_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = entry_count == 1
            ? arc_file.stream.tell()
            : arc_file.stream.read_u32_le();
        entry->name = util::format("%d.bmp", i);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> MgrArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    size_t size_orig = arc_file.stream.read_u32_le();
    size_t size_comp = arc_file.stream.read_u32_le();

    auto data = arc_file.stream.read(size_comp);
    data = decompress(data, size_orig);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<MgrArchiveDecoder>("propeller/mgr");
