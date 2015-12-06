#include "fmt/shiina_rio/s25_image_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/file_from_image.h"

using namespace au;
using namespace au::fmt::shiina_rio;

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

    io::MemoryStream input_stream(input);
    auto left = entry.width;
    while (output_ptr < output_end)
    {
        if (input_stream.tell() & 1)
            input_stream.skip(1);

        const u16 tmp = input_stream.read_u16_le();

        const size_t method = tmp >> 13;
        const size_t skip = (tmp >> 11) & 3;
        input_stream.skip(skip);
        size_t count = tmp & 0x7FF;
        if (!count)
            count = input_stream.read_u32_le();
        if (count > left)
            count = left;
        left -= count;

        if (method == 2)
        {
            if (input_stream.tell() + count * 3 > input_stream.size())
                count = (input_stream.size() - input_stream.tell()) / 3;
            const auto chunk = input_stream.read(3 * count);
            auto chunk_ptr = chunk.get<const u8>();
            for (const auto i : algo::range(count))
            {
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ += 0xFF;
            }
        }

        else if (method == 3)
        {
            const auto chunk = input_stream.read(3);
            for (const auto i : algo::range(count))
            {
                *output_ptr++ += chunk[0];
                *output_ptr++ += chunk[1];
                *output_ptr++ += chunk[2];
                *output_ptr++ += 0xFF;
            }
        }

        else if (method == 4)
        {
            if (input_stream.tell() + count * 4 > input_stream.size())
                count = (input_stream.size() - input_stream.tell()) / 4;
            const auto chunk = input_stream.read(4 * count);
            auto chunk_ptr = chunk.get<const u8>();
            for (const auto i : algo::range(count))
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
            const auto chunk = input_stream.read(4);
            for (const auto i : algo::range(count))
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

static res::Image read_plain(
    io::File &input_file, const ArchiveEntryImpl &entry)
{
    bstr data;
    data.reserve(entry.width * entry.height * 4);

    input_file.stream.seek(entry.offset);
    std::vector<u32> row_offsets(entry.height);
    for (auto &offset : row_offsets)
        offset = input_file.stream.read_u32_le();
    for (const auto y : algo::range(entry.height))
    {
        const auto row_offset = row_offsets[y];
        input_file.stream.seek(row_offset);
        auto row_size = input_file.stream.read_u16_le();
        if (row_offset & 1)
        {
            input_file.stream.skip(1);
            row_size--;
        }
        const auto input_row = input_file.stream.read(row_size);
        const auto output_row = decode_row(input_row, entry);
        data += output_row;
    }

    return res::Image(
        entry.width, entry.height, data, res::PixelFormat::BGRA8888);
}

bool S25ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    S25ImageArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u32_le();
    std::vector<size_t> offsets;
    for (const auto i : algo::range(file_count))
    {
        const auto offset = input_file.stream.read_u32_le();
        if (offset)
            offsets.push_back(offset);
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto offset : offsets)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        input_file.stream.seek(offset);
        entry->width = input_file.stream.read_u32_le();
        entry->height = input_file.stream.read_u32_le();
        input_file.stream.skip(8);
        entry->flags = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.tell();
        if (!entry->width || !entry->height)
            continue;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> S25ImageArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->flags & 0x80000000)
        throw err::NotSupportedError("Flagged S25 images are supported");
    const auto image = read_plain(input_file, *entry);
    return util::file_from_image(image, entry->path);
}

fmt::IDecoder::NamingStrategy S25ImageArchiveDecoder::naming_strategy() const
{
    return NamingStrategy::Sibling;
}

static auto dummy = fmt::register_fmt<S25ImageArchiveDecoder>("shiina-rio/s25");
