#include "fmt/leaf/leafpack_group/leafpack_archive_decoder.h"
#include <boost/algorithm/hex.hpp>
#include "err.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAFPACK"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static void decrypt(bstr &data, const bstr &key)
{
    for (auto i : util::range(data.size()))
        data[i] -= key[i % key.size()];
}

struct LeafpackArchiveDecoder::Priv final
{
    bstr key;
};

LeafpackArchiveDecoder::LeafpackArchiveDecoder() : p(new Priv())
{
}

LeafpackArchiveDecoder::~LeafpackArchiveDecoder()
{
}

bool LeafpackArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.stream.read(magic.size()) == magic;
}

void LeafpackArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--leafpack-key"})
        ->set_value_name("KEY")
        ->set_description("Decryption key");
    ArchiveDecoder::register_cli_options(arg_parser);
}

void LeafpackArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("leafpack-key"))
    {
        std::string key;
        boost::algorithm::unhex(
            arg_parser.get_switch("leafpack-key"), std::back_inserter(key));
        set_key(key);
    }
    ArchiveDecoder::parse_cli_options(arg_parser);
}

void LeafpackArchiveDecoder::set_key(const bstr &key)
{
    p->key = key;
}

std::unique_ptr<fmt::ArchiveMeta>
    LeafpackArchiveDecoder::read_meta_impl(File &arc_file) const
{
    if (p->key.empty())
    {
        throw err::UsageError(
            "File needs key to be unpacked. "
            "Please supply one with --leafpack-key switch.");
    }

    arc_file.stream.seek(magic.size());
    const auto file_count = arc_file.stream.read_u16_le();

    const auto table_size = file_count * 24;
    arc_file.stream.seek(arc_file.stream.size() - table_size);
    auto table_data = arc_file.stream.read(table_size);
    decrypt(table_data, p->key);

    io::MemoryStream table_stream(table_data);
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_stream.read_to_zero(12).str();
        const auto space_pos = entry->name.find_first_of(' ');
        if (space_pos != std::string::npos)
            while (entry->name[space_pos] == ' ')
                entry->name.erase(space_pos, 1);
        if (entry->name.size() > 3)
            entry->name.insert(entry->name.size() - 3, ".");
        entry->offset = table_stream.read_u32_le();
        entry->size = table_stream.read_u32_le();
        table_stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> LeafpackArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    auto data = arc_file.stream.read(entry->size);
    decrypt(data, p->key);
    return std::make_unique<File>(entry->name, data);
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

static auto dummy = fmt::register_fmt<LeafpackArchiveDecoder>("leaf/leafpack");
