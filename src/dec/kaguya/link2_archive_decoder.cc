#include "dec/kaguya/link2_archive_decoder.h"
#include "algo/binary.h"
#include "algo/range.h"
#include "dec/kaguya/common/custom_lzss.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LIN2"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
        u16 type;
    };
}

bool Link2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Link2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        const auto name_size = input_file.stream.read_le<u16>();
        entry->path
            = algo::unxor(input_file.stream.read(name_size), 0xFF).c_str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        entry->type = input_file.stream.read_le<u16>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Link2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);

    bstr data;
    if (entry->type == 1)
    {
        const auto size_orig = input_file.stream.read_le<u32>();
        data = input_file.stream.read(entry->size - 4);
        data = common::custom_lzss_decompress(data, size_orig);
    }
    else
    {
        data = input_file.stream.read(entry->size);
    }

    std::unique_ptr<io::File> output_file = std::make_unique<io::File>(
        entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> Link2ArchiveDecoder::get_linked_formats() const
{
    return {"kaguya/ap", "kaguya/raw-mask", "microsoft/bmp"};
}

static auto _ = dec::register_decoder<Link2ArchiveDecoder>("kaguya/link2");
