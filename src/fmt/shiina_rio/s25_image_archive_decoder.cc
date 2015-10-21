#include "fmt/shiina_rio/s25_image_archive_decoder.h"
#include <boost/filesystem/path.hpp>
#include "err.h"
#include "fmt/naming_strategies.h"
#include "io/buffered_io.h"
#include "util/file_from_grid.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::shiina_rio;
namespace fs = boost::filesystem;

static const bstr magic = "S25\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t width, height;
        size_t offset;
        u32 flags;
    };
}

static bstr decode_row(const bstr &input, const ArchiveEntryImpl &entry)
{
    bstr output(entry.width * 4);
    auto output_ptr = output.get<u8>();
    const auto output_start = output.get<const u8>();
    const auto output_end = output.end<const u8>();

    io::BufferedIO input_io(input);
    auto left = entry.width;
    while (output_ptr < output_end)
    {
        if (input_io.tell() & 1)
            input_io.skip(1);

        const u16 tmp = input_io.read_u16_le();

        const size_t method = tmp >> 13;
        const size_t skip = (tmp >> 11) & 3;
        input_io.skip(skip);
        size_t count = tmp & 0x7FF;
        if (!count)
            count = input_io.read_u32_le();
        if (count > left)
            count = left;
        left -= count;

        if (method == 2)
        {
            if (input_io.tell() + count * 3 > input_io.size())
                count = (input_io.size() - input_io.tell()) / 3;
            const auto chunk = input_io.read(3 * count);
            auto chunk_ptr = chunk.get<const u8>();
            for (const auto i : util::range(count))
            {
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ += 0xFF;
            }
        }

        else if (method == 3)
        {
            const auto chunk = input_io.read(3);
            for (const auto i : util::range(count))
            {
                *output_ptr++ += chunk[0];
                *output_ptr++ += chunk[1];
                *output_ptr++ += chunk[2];
                *output_ptr++ += 0xFF;
            }
        }

        else if (method == 4)
        {
            if (input_io.tell() + count * 4 > input_io.size())
                count = (input_io.size() - input_io.tell()) / 4;
            const auto chunk = input_io.read(4 * count);
            auto chunk_ptr = chunk.get<const u8>();
            for (const auto i : util::range(count))
            {
                *output_ptr++ = chunk_ptr[1];
                *output_ptr++ = chunk_ptr[2];
                *output_ptr++ = chunk_ptr[3];
                *output_ptr++ = chunk_ptr[0];
                chunk_ptr += 4;
            }
        }

        else if (method == 5)
        {
            const auto chunk = input_io.read(4);
            for (const auto i : util::range(count))
            {
                *output_ptr++ += chunk[1];
                *output_ptr++ += chunk[2];
                *output_ptr++ += chunk[3];
                *output_ptr++ += chunk[0];
            }
        }

        else
            output_ptr += count * 4;
    }

    return output;
}

static pix::Grid read_plain(File &arc_file, const ArchiveEntryImpl &entry)
{
    bstr data;
    data.reserve(entry.width * entry.height * 4);

    arc_file.io.seek(entry.offset);
    std::vector<u32> row_offsets(entry.height);
    for (auto &offset : row_offsets)
        offset = arc_file.io.read_u32_le();
    for (const auto y : util::range(entry.height))
    {
        const auto row_offset = row_offsets[y];
        arc_file.io.seek(row_offset);
        auto row_size = arc_file.io.read_u16_le();
        if (row_offset & 1)
        {
            arc_file.io.skip(1);
            row_size--;
        }
        const auto input_row = arc_file.io.read(row_size);
        const auto output_row = decode_row(input_row, entry);
        data += output_row;
    }

    return pix::Grid(entry.width, entry.height, data, pix::Format::BGRA8888);
}

bool S25ImageArchiveDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    S25ImageArchiveDecoder::read_meta_impl(File &arc_file) const
{
    const auto base_name = fs::path(arc_file.name).stem().string();

    arc_file.io.seek(magic.size());
    const auto file_count = arc_file.io.read_u32_le();
    std::vector<size_t> offsets;
    for (const auto i : util::range(file_count))
    {
        const auto offset = arc_file.io.read_u32_le();
        if (offset)
            offsets.push_back(offset);
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto offset : offsets)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        arc_file.io.seek(offset);
        entry->width = arc_file.io.read_u32_le();
        entry->height = arc_file.io.read_u32_le();
        arc_file.io.skip(8);
        entry->flags = arc_file.io.read_u32_le();
        entry->offset = arc_file.io.tell();
        entry->name = offsets.size() > 1
            ? util::format("%s_%03d", base_name.c_str(), meta->entries.size())
            : base_name;
        if (!entry->width || !entry->height)
            continue;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> S25ImageArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->flags & 0x80000000)
        throw err::NotSupportedError("Flagged S25 images are supported");
    const auto image = read_plain(arc_file, *entry);
    return util::file_from_grid(image, e.name);
}

std::unique_ptr<fmt::INamingStrategy>
    S25ImageArchiveDecoder::naming_strategy() const
{
    return std::make_unique<SiblingNamingStrategy>();
}

static auto dummy = fmt::register_fmt<S25ImageArchiveDecoder>("shiina-rio/s25");
