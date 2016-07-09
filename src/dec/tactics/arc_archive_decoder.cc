#include "dec/tactics/arc_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::tactics;

static const bstr magic = "TACTICS_ARC_FILE"_b;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        bstr key;
    };
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v0(io::File &input_file)
{
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>();
    if (size_comp > 1024 * 1024 * 10)
        throw err::BadDataSizeError();

    input_file.stream.skip(4);
    const auto table_data
        = algo::unxor(input_file.stream.read(size_comp), 0xFF);
    io::MemoryStream table_stream(
        algo::pack::lzss_decompress(table_data, size_orig));

    const auto data_start = input_file.stream.pos();
    const auto key = table_stream.read_to_zero();
    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->offset = table_stream.read_le<u32>() + data_start;
        entry->size_comp = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        entry->key = key;
        auto name_size = table_stream.read_le<u32>();

        table_stream.skip(8);
        entry->path = algo::sjis_to_utf8(table_stream.read(name_size)).str();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v1(io::File &input_file)
{
    auto meta = std::make_unique<dec::ArchiveMeta>();
    while (input_file.stream.left())
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->size_comp = input_file.stream.read_le<u32>();
        if (!entry->size_comp)
            break;
        entry->size_orig = input_file.stream.read_le<u32>();
        const auto name_size = input_file.stream.read_le<u32>();
        input_file.stream.skip(8);
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read(name_size)).str();
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(entry->size_comp);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

ArcArchiveDecoder::ArcArchiveDecoder()
{
    add_arg_parser_decorator(
        ArgParserDecorator(
            [this](ArgParser &arg_parser)
            {
                auto sw = arg_parser.register_switch({"arc-key"})
                    ->set_value_name("KEY")
                    ->set_description("key used to decrypt the files");
            },
            [this](const ArgParser &arg_parser)
            {
                if (arg_parser.has_switch("arc-key"))
                    key = arg_parser.get_switch("arc-key");
            }));
}

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    std::vector<std::function<std::unique_ptr<dec::ArchiveMeta>(io::File &)>>
        meta_readers
        {
            read_meta_v0,
            read_meta_v1
        };

    for (const auto meta_reader : meta_readers)
    {
        input_file.stream.seek(magic.size());
        try
        {
            auto meta = meta_reader(input_file);
            if (!key.empty())
            {
                for (auto &entry : meta->entries)
                    static_cast<CustomArchiveEntry*>(entry.get())->key = key;
            }
            return meta;
        }
        catch (const std::exception)
        {
            continue;
        }
    }

    throw err::NotSupportedError("Archive is encrypted in unknown way.");
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    if (entry->key.size())
        data = algo::unxor(data, entry->key);
    if (entry->size_orig)
        data = algo::pack::lzss_decompress(data, entry->size_orig);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"microsoft/dds"};
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("tactics/arc");
