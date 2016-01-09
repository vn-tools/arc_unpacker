#include "dec/leaf/leafpack_group/leafpack_archive_decoder.h"
#include "algo/range.h"
#include "algo/str.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "LEAFPACK"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static void decrypt(bstr &data, const bstr &key)
{
    for (auto i : algo::range(data.size()))
        data[i] -= key[i % key.size()];
}

LeafpackArchiveDecoder::LeafpackArchiveDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--leafpack-key"})
                ->set_value_name("KEY")
                ->set_description("Decryption key");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_switch("leafpack-key"))
                key = algo::unhex(arg_parser.get_switch("leafpack-key"));
        });
}

bool LeafpackArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> LeafpackArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    if (key.empty())
    {
        throw err::UsageError(
            "File needs key to be unpacked. "
            "Please supply one with --leafpack-key switch.");
    }

    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u16>();

    const auto table_size = file_count * 24;
    input_file.stream.seek(input_file.stream.size() - table_size);
    auto table_data = input_file.stream.read(table_size);
    decrypt(table_data, key);

    io::MemoryStream table_stream(table_data);
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto name = table_stream.read_to_zero(12).str();
        const auto space_pos = name.find_first_of(' ');
        if (space_pos != std::string::npos)
            while (name[space_pos] == ' ')
                name.erase(space_pos, 1);
        if (name.size() > 3)
            name.insert(name.size() - 3, ".");
        entry->path = name;
        entry->offset = table_stream.read_le<u32>();
        entry->size = table_stream.read_le<u32>();
        table_stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> LeafpackArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    decrypt(data, key);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> LeafpackArchiveDecoder::get_linked_formats() const
{
    return
    {
        "leaf/lc3",
        "leaf/lf2",
        "leaf/lf3",
        "leaf/lfb",
        "leaf/lfg",
        "leaf/p16",
    };
}

static auto _ = dec::register_decoder<LeafpackArchiveDecoder>("leaf/leafpack");
