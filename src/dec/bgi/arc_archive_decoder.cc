#include "dec/bgi/arc_archive_decoder.h"
#include <map>
#include "algo/range.h"

using namespace au;
using namespace au::dec::bgi;

namespace
{
    class ArcArchiveType final
    {
    public:
        const size_t path_size;
        const size_t skip_size;

        size_t get_header_size() const;
    };
}

size_t ArcArchiveType::get_header_size() const
{
    return this->skip_size + this->path_size + 2 * 4;
}

static const std::map<bstr, ArcArchiveType> types =
{
    {"PackFile\x20\x20\x20\x20"_b, {16, 8}},
    {"BURIKO ARC20"_b, {96, 24}}
};

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    // All magics have the same size for now so finding the type is easier
    return types.find(input_file.stream.read(12)) != types.end();
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto type = types.at(input_file.stream.read(12));

    const auto file_count = input_file.stream.read_le<u32>();
    const auto file_data_start = input_file.stream.pos() +
        file_count * type.get_header_size();

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = input_file.stream.read_to_zero(type.path_size).str();
        entry->offset = input_file.stream.read_le<u32>() + file_data_start;
        entry->size = input_file.stream.read_le<u32>();
        input_file.stream.skip(type.skip_size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"bgi/cbg", "bgi/dsc", "bgi/audio"};
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("bgi/arc");
