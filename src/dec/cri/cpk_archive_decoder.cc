#include "dec/cri/cpk_archive_decoder.h"
#include <map>
#include "algo/range.h"
#include "algo/str.h"
#include "err.h"
#include "io/memory_stream.h"
#include "io/msb_bit_reader.h"
#include "util/any.h"

using namespace au;
using namespace au::dec::cri;

static const bstr magic = "CPK\x20"_b;
static const bstr layla_magic = "CRILAYLA"_b;

static const u32 storage_mask    = 0xF0;
static const u32 storage_none    = 0x00;
static const u32 storage_zero    = 0x10;
static const u32 storage_const   = 0x30;
static const u32 storage_per_row = 0x50;

static const u32 type_mask = 0x0F;
static const u32 type_u8a  = 0x00;
static const u32 type_u8b  = 0x01;
static const u32 type_u16a = 0x02;
static const u32 type_u16b = 0x03;
static const u32 type_u32a = 0x04;
static const u32 type_u32b = 0x05;
static const u32 type_u64a = 0x06;
static const u32 type_u64b = 0x07;
static const u32 type_f32  = 0x08;
static const u32 type_str  = 0x0A;
static const u32 type_data = 0x0B;

namespace
{
    struct TocEntry final
    {
        u32 id;
        std::string dir_name;
        std::string file_name;
        size_t file_offset;
        size_t file_size;
        size_t extract_size;
        std::string user_string;

        std::string local_dir;
        u64 mtime;
    };

    using Cell = util::any;
    using Row = std::map<std::string, Cell>;
    using Toc = std::map<u32, TocEntry>;

    struct Column final
    {
        u32 flags;
        std::string name;
    };

    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static bstr decrypt_utf_packet(const bstr &input)
{
    u32 m = 0x655F;
    bstr output(input);
    for (auto &c : output)
    {
        c ^= m & 0xFF;
        m *= 0x4115;
    }
    return output;
}

static bstr read_utf_packet(io::IStream &input_stream)
{
    bool isUtfEncrypted = false;
    input_stream.skip(4);
    const auto utf_size = input_stream.read_u64_le();
    const auto utf_packet = input_stream.read(utf_size);
    return utf_packet.substr(0, 4) == "@UTF"_b
        ? utf_packet
        : decrypt_utf_packet(utf_packet);
}

static bstr decompress_layla(const bstr &input)
{
    io::MemoryStream input_stream(input);
    input_stream.seek(layla_magic.size());
    const auto size_orig = input_stream.read_u32_le();
    const auto size_comp = input_stream.read_u32_le();
    const auto data_comp = algo::reverse(input_stream.read(size_comp));
    const auto prefix = input_stream.read_to_eof();

    io::MsbBitReader bit_reader(data_comp);
    bstr output;
    output.reserve(size_orig);
    while (output.size() < size_orig)
    {
        if (bit_reader.get(1))
        {
            auto repetitions = 3;
            auto look_behind = bit_reader.get(13) + 3;

            std::vector<size_t> sizes = {5, 3, 2};
            while (true)
            {
                size_t size = 8;
                if (!sizes.empty())
                {
                    size = sizes.back();
                    sizes.pop_back();
                }
                const auto marker = bit_reader.get(size);
                repetitions += marker;
                if (marker != (1u << size) - 1)
                    break;
            }

            while (repetitions--)
                output += output.at(output.size() - look_behind);
        }
        else
            output += static_cast<u8>(bit_reader.get(8));
    }

    return prefix + algo::reverse(output);
}

static std::vector<Row> parse_utf_packet(const bstr &utf_packet)
{
    io::MemoryStream utf_stream(utf_packet);
    if (utf_stream.read(4) != "@UTF"_b)
        throw err::CorruptDataError("Expected UTF packet");
    const auto table_size = utf_stream.read_u32_be();
    const auto rows_offset_base = utf_stream.read_u32_be() + 8;
    const auto text_offset_base = utf_stream.read_u32_be() + 8;
    const auto data_offset_base = utf_stream.read_u32_be() + 8;
    const auto table_name_offset = utf_stream.read_u32_be();
    const auto column_count = utf_stream.read_u16_be();
    const auto row_size = utf_stream.read_u16_be();
    const auto row_count = utf_stream.read_u32_be();

    std::string table_name;
    utf_stream.peek(
        text_offset_base + table_name_offset,
        [&]() { table_name = utf_stream.read_to_zero().str(); });

    std::vector<Column> columns(column_count);
    for (auto &column : columns)
    {
        column.flags = utf_stream.read_u8();
        if (column.flags == 0)
            column.flags = utf_stream.read_u32_be();

        const auto column_name_offset = utf_stream.read_u32_be();
        utf_stream.peek(
            text_offset_base + column_name_offset,
            [&]() { column.name = utf_stream.read_to_zero().str(); });
    }

    std::vector<Row> rows(row_count);
    for (const auto y : algo::range(row_count))
    {
        auto &row = rows[y];
        utf_stream.seek(rows_offset_base + y * row_size);
        for (const auto &column : columns)
        {
            auto &cell = row[column.name];

            const auto storage_type = column.flags & storage_mask;
            if (storage_type == storage_none
                || storage_type == storage_zero
                || storage_type == storage_const)
            {
                continue;
            }

            const auto type = column.flags & type_mask;
            switch (type)
            {
                case type_u8a:
                case type_u8b:
                    cell = utf_stream.read_u8();
                    break;

                case type_u16a:
                case type_u16b:
                    cell = utf_stream.read_u16_be();
                    break;

                case type_u32a:
                case type_u32b:
                    cell = utf_stream.read_u32_be();
                    break;

                case type_u64a:
                case type_u64b:
                    cell = utf_stream.read_u64_be();
                    break;

                case type_f32:
                    cell = utf_stream.read_f32_be();
                    break;

                case type_str:
                {
                    const auto value_offset = utf_stream.read_u32_be();
                    utf_stream.peek(
                        text_offset_base + value_offset,
                        [&]()
                        {
                            cell = utf_stream.read_to_zero().str();
                        });
                    break;
                }

                case type_data:
                {
                    const auto data_offset = utf_stream.read_u32_be();
                    const auto data_size = utf_stream.read_u32_be();
                    utf_stream.peek(
                        data_offset_base + data_offset,
                        [&]() { cell = utf_stream.read(data_size); });
                    break;
                }
            }
        }
        rows.push_back(row);
    }

    return rows;
}

static void read_toc(
    io::IStream &input_stream,
    const size_t toc_offset,
    const size_t content_offset,
    Toc &toc)
{
    const auto data_offset_base = std::min<size_t>(content_offset, toc_offset);

    input_stream.seek(toc_offset);
    if (input_stream.read(4) != "TOC\x20"_b)
        throw err::CorruptDataError("Expected TOC packet");

    const auto table = parse_utf_packet(read_utf_packet(input_stream));
    for (const auto &row : table)
    {
        TocEntry entry;
        entry.id = row.at("ID").get<u32>();
        if (row.at("DirName"))
            entry.dir_name = row.at("DirName").get<std::string>();
        entry.file_name = row.at("FileName").get<std::string>();
        entry.file_offset = row.at("FileOffset").get<u64>() + data_offset_base;
        entry.file_size = row.at("FileSize").get<u32>();
        if (row.at("ExtractSize"))
            entry.extract_size = row.at("ExtractSize").get<u32>();
        if (row.at("UserString"))
            entry.user_string = row.at("UserString").get<std::string>();
        toc[entry.id] = entry;
    }
}

static void read_etoc(
    io::IStream &input_stream, const size_t etoc_offset, Toc &toc)
{
    input_stream.seek(etoc_offset);
    if (input_stream.read(4) != "ETOC"_b)
        throw err::CorruptDataError("Expected ETOC packet");

    const auto table = parse_utf_packet(read_utf_packet(input_stream));
    for (const auto i : algo::range(toc.size()))
    {
        auto &entry = toc[i];
        const auto &row = table.at(i);
        if (row.at("LocalDir"))
            entry.local_dir = row.at("LocalDir").get<std::string>();
        entry.mtime = row.at("UpdateDateTime").get<u64>();
    }
}

static void read_itoc(
    io::IStream &input_stream,
    const size_t itoc_offset,
    const size_t content_offset,
    const size_t align,
    Toc &toc)
{
    input_stream.seek(itoc_offset);
    if (input_stream.read(4) != "ITOC"_b)
        throw err::CorruptDataError("Expected ITOC packet");

    const auto table = parse_utf_packet(read_utf_packet(input_stream));
    if (table.empty())
        return;

    if (table[0].find("DataL") != table[0].end())
    {
        const auto data_l = parse_utf_packet(table[0].at("DataL").get<bstr>());
        const auto data_h = parse_utf_packet(table[0].at("DataH").get<bstr>());
        for (const auto &row : data_l)
        {
            const auto entry_id = row.at("ID").get<u16>();
            toc[entry_id].file_size = row.at("FileSize").get<u16>();
            if (row.at("ExtractSize"))
                toc[entry_id].extract_size = row.at("ExtractSize").get<u16>();
        }
        for (const auto &row : data_h)
        {
            const auto entry_id = row.at("ID").get<u16>();
            toc[entry_id].file_size = row.at("FileSize").get<u32>();
            if (row.at("ExtractSize"))
                toc[entry_id].extract_size = row.at("ExtractSize").get<u32>();
        }
        size_t offset = content_offset;
        std::vector<u32> ids;
        for (const auto &kv : toc)
            ids.push_back(kv.first);
        std::sort(ids.begin(), ids.end());
        for (const auto id : ids)
        {
            const auto size = toc[id].file_size;
            toc[id].file_offset = offset;
            offset += size;
            if (size % align)
                offset += align - (size % align);
        }
        return;
    }

    for (const auto &row : table)
    {
        const auto id = row.at("ID").get<u32>();
        const auto idx = row.at("TocIndex").get<u32>();
    }
}

bool CpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> CpkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto header_packet = read_utf_packet(input_file.stream);
    const auto header = parse_utf_packet(header_packet).at(0);
    const auto content_offset = header.at("ContentOffset").get<u64>();
    const auto align = header.at("Align").get<u16>();
    Toc toc;

    if (header.at("TocOffset"))
    {
        read_toc(
            input_file.stream,
            header.at("TocOffset").get<u64>(),
            content_offset,
            toc);
    }

    if (header.at("ItocOffset"))
    {
        read_itoc(
            input_file.stream,
            header.at("ItocOffset").get<u64>(),
            content_offset,
            align,
            toc);
    }

    if (header.at("EtocOffset"))
        read_etoc(input_file.stream, header.at("EtocOffset").get<u64>(), toc);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto &kv : toc)
    {
        const auto &toc_entry = kv.second;
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = toc_entry.dir_name;
        entry->path /= toc_entry.file_name;
        entry->offset = toc_entry.file_offset;
        entry->size = toc_entry.file_size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> CpkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream
        .seek(entry->offset)
        .read(entry->size);
    if (data.substr(0, layla_magic.size()) == layla_magic)
        data = decompress_layla(data);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> CpkArchiveDecoder::get_linked_formats() const
{
    return {"cri/hca", "playstation/gxt"};
}

static auto _ = dec::register_decoder<CpkArchiveDecoder>("cri/cpk");
