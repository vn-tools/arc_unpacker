#include "dec/nsystem/fjsys_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::nsystem;

static const bstr magic = "FJSYS\x00\x00\x00"_b;

bool FjsysArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> FjsysArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto header_size = input_file.stream.read_le<u32>();
    const auto file_names_size = input_file.stream.read_le<u32>();
    const auto file_names_start = header_size - file_names_size;
    const auto file_count = input_file.stream.read_le<u32>();
    input_file.stream.skip(64);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        const auto file_name_offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u64>();
        input_file.stream.peek(file_name_offset + file_names_start, [&]()
        {
            entry->path = input_file.stream.read_to_zero().str();
        });
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> FjsysArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> FjsysArchiveDecoder::get_linked_formats() const
{
    return {"nsystem/mgd"};
}

static auto _ = dec::register_decoder<FjsysArchiveDecoder>("nsystem/fjsys");
