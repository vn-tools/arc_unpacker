#include "dec/kaguya/wfl_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "WFL1"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };
}

bool WflArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> WflArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto meta = std::make_unique<ArchiveMeta>();
    while (input_file.stream.left() >= 4)
    {
        const auto name_size = input_file.stream.read_le<u32>();
        if (!name_size)
            break;
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = algo::sjis_to_utf8(algo::unxor(
            input_file.stream.read(name_size), 0xFF).str()).str(true);
        const auto type = input_file.stream.read_le<u16>();
        entry->size_comp = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.tell();
        entry->size_orig = type == 1
            ? input_file.stream.read_le<u32>()
            : entry->size_comp;
        input_file.stream.skip(entry->size_comp);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> WflArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto output_file = std::make_unique<io::File>(
        entry->path,
        input_file.stream.seek(entry->offset).read(entry->size_comp));
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<WflArchiveDecoder>("kaguya/wfl");
