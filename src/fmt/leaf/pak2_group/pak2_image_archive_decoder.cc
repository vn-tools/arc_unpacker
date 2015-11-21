#include "fmt/leaf/pak2_group/pak2_image_archive_decoder.h"
#include <boost/filesystem/path.hpp>
#include "err.h"
#include "fmt/naming_strategies.h"
#include "util/file_from_grid.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;
namespace fs = boost::filesystem;

static const bstr magic = "\x5F\xF8\x6D\x75"_b;
static const bstr mask_magic = "\x03\xC5\x0D\xA6"_b;
static const bstr end_magic = "\xAF\xF6\x4D\x4E"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t color_offset, mask_offset, size;
        size_t width, height, bpp;
    };
}

bool Pak2ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(4).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Pak2ImageArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();

    input_file.stream.seek(0);
    ArchiveEntryImpl *last_entry = nullptr;
    while (!input_file.stream.eof())
    {
        input_file.stream.skip(4);
        const auto entry_magic = input_file.stream.read(4);
        if (entry_magic == magic)
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            input_file.stream.skip(18);
            entry->bpp = input_file.stream.read_u16_le();
            input_file.stream.skip(8);
            entry->width = input_file.stream.read_u16_le();
            entry->height = input_file.stream.read_u16_le();
            entry->color_offset = input_file.stream.tell();
            entry->size = entry->width * entry->height * entry->bpp >> 3;
            input_file.stream.skip(entry->size);

            if (!entry->width || !entry->height)
                continue;
            last_entry = entry.get();
            meta->entries.push_back(std::move(entry));
        }
        else if (entry_magic == mask_magic)
        {
            if (!last_entry)
                throw err::CorruptDataError("Mask found, but no color data");
            input_file.stream.skip(28);
            last_entry->mask_offset = input_file.stream.tell();
            input_file.stream.skip(last_entry->width * last_entry->height);
        }
        else if (entry_magic == end_magic)
        {
            input_file.stream.skip(12);
            if (!input_file.stream.eof())
                throw err::CorruptDataError("More data follow");
        }
        else
            throw err::CorruptDataError("Unexpected magic");
    }

    const auto base_name = fs::path(input_file.name).stem().string();
    for (auto i : util::range(meta->entries.size()))
        meta->entries[i]->name = meta->entries.size() > 1
            ? util::format("%s_%03d", base_name.c_str(), i)
            : base_name;

    return meta;
}

std::unique_ptr<io::File> Pak2ImageArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    pix::Format format;
    if (entry->bpp == 8)
        format = pix::Format::Gray8;
    else if (entry->bpp == 16)
        format = pix::Format::BGRnA5551;
    else
        throw err::UnsupportedBitDepthError(entry->bpp);

    const auto data = input_file.stream
        .seek(entry->color_offset)
        .read(entry->size);
    auto image = pix::Grid(entry->width, entry->height, data, format);
    image.flip_vertically();
    if (entry->mask_offset)
    {
        const auto mask = input_file
            .stream.seek(entry->mask_offset)
            .read(entry->width * entry->height);
        const auto mask_image
            = pix::Grid(entry->width, entry->height, mask, pix::Format::Gray8);
        image.apply_mask(mask_image);
    }
    return util::file_from_grid(image, entry->name);
}

std::unique_ptr<fmt::INamingStrategy>
    Pak2ImageArchiveDecoder::naming_strategy() const
{
    return std::make_unique<SiblingNamingStrategy>();
}

static auto dummy
    = fmt::register_fmt<Pak2ImageArchiveDecoder>("leaf/pak2-image");
