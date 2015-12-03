#include "fmt/yuris/ypf_archive_decoder.h"
#include <set>
#include "err.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::yuris;

static const bstr magic = "YPF\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        u8 type;
        bool compressed;
        size_t offset;
        size_t size_orig;
        size_t size_comp;
    };
}

static u8 get_name_size(io::Stream &input_stream, size_t initial_pos)
{
    const auto byte = input_stream.read_u8() ^ 0xFF;
    static const std::vector<u8> table =
        {
            0x03, 0x48, 0x06, 0x35, 0x0C, 0x10, 0x11, 0x19,
            0x1C, 0x1E, 0x09, 0x0B, 0x0D, 0x13, 0x15, 0x1B,
            0x20, 0x23, 0x26, 0x29, 0x2C, 0x2F, 0x2E, 0x32,
        };

    const auto it = std::find(table.begin() + initial_pos, table.end(), byte);
    if (it == table.end())
        return byte;
    const auto pos = it - table.begin();
    return table[pos + ((pos & 1) ? -1 : 1)];
}

static bstr unxor(const bstr &input, const u8 key)
{
    bstr output(input);
    for (auto &c : output)
        c ^= key;
    return output;
}

static size_t guess_name_crypt_pos(
    io::Stream &table_stream,
    const size_t version,
    const size_t file_count)
{
    for (const auto initial_pos : {4, 0, 10})
    {
        table_stream.seek(0);
        try
        {
            for (const auto i : util::range(file_count))
            {
                table_stream.skip(4);
                const auto name_size = get_name_size(table_stream, initial_pos);
                table_stream.skip(name_size);
                table_stream.skip(14);
                table_stream.skip(version == 0xDE ? 12 : 4);
            }
            table_stream.seek(0);
            return initial_pos;
        }
        catch (...)
        {
            continue;
        }
    }
    throw err::NotSupportedError("Failed to guess the name crypt position");
}

static u8 guess_key(const std::vector<bstr> &names)
{
    static const std::set<std::string> good_extensions
        = {"bmp", "png", "ogg", "wav", "txt", "ybn"};

    for (const auto &name : names)
    {
        if (name.size() < 4)
            continue;
        const auto key = name.at(name.size() - 4) ^ '.';
        const auto decoded_name = unxor(name, key);
        const auto possible_extension = decoded_name.substr(-3).str();
        if (good_extensions.find(possible_extension) != good_extensions.end())
            return key;
    }
    throw err::NotSupportedError("Failed to guess the key");
}

bool YpfArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    YpfArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(4);
    const auto version = input_file.stream.read_u32_le();
    const auto file_count = input_file.stream.read_u32_le();
    const auto table_size = input_file.stream.read_u32_le();

    io::MemoryStream table_stream(
        input_file.stream.seek(0x20).read(table_size));

    const auto name_crypt_pos
        = guess_name_crypt_pos(table_stream, version, file_count);

    std::vector<bstr> names;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : util::range(file_count))
    {
        table_stream.skip(4);

        const auto name_size = get_name_size(table_stream, name_crypt_pos);
        const auto name = table_stream.read(name_size);
        names.push_back(name);

        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->type = table_stream.read_u8();
        entry->compressed = table_stream.read_u8() == 1;
        entry->size_orig = table_stream.read_u32_le();
        entry->size_comp = table_stream.read_u32_le();
        entry->offset = table_stream.read_u32_le();
        table_stream.skip(version == 0xDE ? 12 : 4);
        meta->entries.push_back(std::move(entry));
    }

    const auto key = guess_key(names);
    for (const auto i : util::range(file_count))
        meta->entries[i]->path = util::sjis_to_utf8(unxor(names[i], key)).str();
    return meta;
}

std::unique_ptr<io::File> YpfArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    if (entry->compressed)
        data = util::pack::zlib_inflate(data);
    return std::make_unique<io::File>(entry->path, data);
}

static auto dummy = fmt::register_fmt<YpfArchiveDecoder>("yuris/ypf");
