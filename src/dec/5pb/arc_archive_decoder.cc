#include "dec/5pb/arc_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::_5pb;

static const std::string read_string(io::BaseByteStream &input_stream)
{
    bstr str;
    while (input_stream.left())
    {
        const auto chunk = input_stream.read(2);
        if (chunk == "\x00\x00"_b)
            break;
        str += chunk;
    }
    return algo::utf16_to_utf8(str).str();
}

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();
    for (const auto i : algo::range(file_count - 1))
    {
        input_file.stream.skip(8);
        while (true)
        {
            const auto c1 = input_file.stream.read<u8>();
            const auto c2 = input_file.stream.read<u8>();
            if (!c1 && !c2)
                break;
        }
    }
    const auto last_file_offset
        = input_file.stream.read_le<u32>() + 8 + table_size;
    const auto last_file_size = input_file.stream.read_le<u32>();
    return last_file_offset + last_file_size == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u32>() + 8 + table_size;
        entry->path = read_string(input_file.stream);
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
    return {"5pb/pnap"};
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("5pb/arc");
