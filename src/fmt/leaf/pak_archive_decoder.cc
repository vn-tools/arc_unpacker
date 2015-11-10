#include "fmt/leaf/pak_archive_decoder.h"
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <map>
#include "err.h"
#include "fmt/leaf/grp_image_decoder.h"
#include "util/encoding.h"
#include "util/file_from_grid.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool compressed;
        bool already_unpacked;
    };
}

// Modified LZSS routine
// - starting position at 0 rather than 0xFEE
// - optionally, additional byte for repetition count
// - dictionary writing in two passes
static bstr custom_lzss_decompress(
    const bstr &input, size_t output_size, const size_t dict_capacity)
{
    std::vector<u8> dict(dict_capacity);
    size_t dict_size = 0;
    size_t dict_pos = 0;

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();
    auto input_ptr = input.get<const u8>();
    auto input_end = input.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end)
    {
        control >>= 1;
        if (!(control & 0x100))
            control = *input_ptr++ | 0xFF00;

        if (control & 1)
        {
            dict[dict_pos++] = *output_ptr++ = *input_ptr++;
            dict_pos %= dict_capacity;
            if (dict_size < dict_capacity)
                dict_size++;
        }
        else
        {
            auto tmp = *reinterpret_cast<const u16*>(input_ptr);
            input_ptr += 2;

            auto look_behind_pos = tmp >> 4;
            auto repetitions = tmp & 0xF;
            if (repetitions == 0xF)
                repetitions += *input_ptr++;
            repetitions += 3;

            auto i = repetitions;
            while (i-- && output_ptr < output_end)
            {
                *output_ptr++ = dict[look_behind_pos++];
                look_behind_pos %= dict_size;
            }

            auto source = &output_ptr[-repetitions];
            while (source < output_ptr)
            {
                dict[dict_pos++] = *source++;
                dict_pos %= dict_capacity;
                if (dict_size < dict_capacity)
                    dict_size++;
            }
        }
    }

    return output;
}

struct PakArchiveDecoder::Priv final
{
    boost::optional<int> version;
};

PakArchiveDecoder::PakArchiveDecoder() : p(new Priv())
{
}

PakArchiveDecoder::~PakArchiveDecoder()
{
}

void PakArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--pak-version"})
        ->set_value_name("NUMBER")
        ->set_description("File version (1 or 2)");
    ArchiveDecoder::register_cli_options(arg_parser);
}

void PakArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("pak-version"))
    {
        set_version(boost::lexical_cast<int>(
            arg_parser.get_switch("pak-version")));
    }
    ArchiveDecoder::parse_cli_options(arg_parser);
}

void PakArchiveDecoder::set_version(const int version)
{
    if (version != 1 && version != 2)
        throw err::UsageError("PAK version can be either '1' or '2'");
    p->version = version;
}

bool PakArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    auto meta = read_meta(arc_file);
    if (!meta->entries.size())
        return false;
    auto last_entry = static_cast<const ArchiveEntryImpl*>(
        meta->entries[meta->entries.size() - 1].get());
    return last_entry->offset + last_entry->size == arc_file.io.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto file_count = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->already_unpacked = false;
        entry->name = util::sjis_to_utf8(arc_file.io.read_to_zero(16)).str();
        entry->size = arc_file.io.read_u32_le();
        entry->compressed = arc_file.io.read_u32_le() > 0;
        entry->offset = arc_file.io.read_u32_le();
        if (entry->size)
            meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PakArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    if (!p->version)
    {
        throw err::UsageError(
            "Please choose PAK version with --pak-version switch.");
    }

    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->already_unpacked)
        return nullptr;

    arc_file.io.seek(entry->offset);
    bstr data;
    if (entry->compressed)
    {
        auto size_comp = arc_file.io.read_u32_le();
        auto size_orig = arc_file.io.read_u32_le();
        data = arc_file.io.read(size_comp - 8);
        data = custom_lzss_decompress(
            data, size_orig, p->version == 1 ? 0x1000 : 0x800);
    }
    else
    {
        data = arc_file.io.read(entry->size);
    }

    return std::make_unique<File>(entry->name, data);
}

void PakArchiveDecoder::preprocess(
    File &arc_file, fmt::ArchiveMeta &meta, const FileSaver &saver) const
{
    std::map<std::string, ArchiveEntryImpl*> palette_entries, sprite_entries;
    for (auto &entry : meta.entries)
    {
        auto fn = entry->name.substr(0, entry->name.find_first_of('.'));
        if (entry->name.find("c16") != std::string::npos)
            palette_entries[fn] = static_cast<ArchiveEntryImpl*>(entry.get());
        else if (entry->name.find("grp") != std::string::npos)
            sprite_entries[fn] = static_cast<ArchiveEntryImpl*>(entry.get());
    }

    GrpImageDecoder grp_image_decoder;
    for (auto it : sprite_entries)
    {
        try
        {
            auto &sprite_entry = it.second;
            auto &palette_entry = palette_entries.at(it.first);
            auto sprite_file = read_file(arc_file, meta, *sprite_entry);
            auto palette_file = read_file(arc_file, meta, *palette_entry);
            auto sprite = grp_image_decoder.decode(*sprite_file, *palette_file);
            saver.save(util::file_from_grid(sprite, sprite_entry->name));
            sprite_entry->already_unpacked = true;
            palette_entry->already_unpacked = true;
        }
        catch (...)
        {
            continue;
        }
    }
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return { "leaf/grp" };
}

static auto dummy = fmt::register_fmt<PakArchiveDecoder>("leaf/pak");
