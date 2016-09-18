#include "dec/cyberworks/arc_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "dec/cyberworks/dat_image_decoder.h"
#include "enc/png/png_image_encoder.h"
#include "io/memory_byte_stream.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::cyberworks;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        std::vector<std::unique_ptr<io::File>> data_files;
        DatPlugin plugin;
    };

    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        bstr type;
        u32 data_file_id;
    };
}

static u32 read_obfuscated_number(io::BaseByteStream &input_stream)
{
    u32 ret = 0;
    for (const auto i : algo::range(8))
    {
        ret *= 10;
        const auto byte = input_stream.read<u8>();
        if (byte != 0xFF)
            ret += byte ^ 0x7F;
    }
    return ret;
}

static std::vector<std::string> get_data_file_names(
    const io::File &input_file, const DatPlugin &plugin)
{
    const auto toc_file_name = input_file.path.name();
    const auto tmp = plugin.toc_to_data_file_name_map.find(toc_file_name);
    if (tmp == plugin.toc_to_data_file_name_map.end())
    {
        throw err::RecognitionError(
            "Unrecognized TOC file name, cannot proceed.");
    }
    return tmp->second;
}

static void decode_tinkerbell_data_headerless(bstr &data)
{
    const auto key =
        "DBB3206F-F171-4885-A131-EC7FBA6FF491 Copyright 2004 "
        "Cyberworks \"TinkerBell\"., all rights reserved.\x00"_b;
    size_t key_pos = 0;
    for (const auto i : algo::range(4, data.size()))
    {
        if (i >= 0xE1F)
            break;
        data[i] ^= key[key_pos];
        key_pos++;
        if (key_pos == key.size())
            key_pos = 1;
    }
    data[0] = 'O';
    data[1] = 'g';
    data[2] = 'g';
    data[3] = 'S';
}

static void decode_tinkerbell_data_with_header(bstr &data)
{
    data = data.substr(12);
    decode_tinkerbell_data_headerless(data);
}

ArcArchiveDecoder::ArcArchiveDecoder()
{
    plugin_manager.add(
        "aniyome-kyouka",
        "Aniyome Kyouka-san to Sono Haha Chikako-san "
            "~Bijin Tsuma to Bijukubo to Issho~",
        {
            {
                {"dPih.dat", {"dPi.dat"}},
                {"dSch.dat", {"dSc.dat"}},
                {"dSo.dat", {"dSoh.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {5, 7, 0, 6, 4, 3, 1},
            true,
        });

    plugin_manager.add(
        "zoku-etsuraku",
        "Zoku Etsuraku no Tane",
        {
            {
                {"Arc01.dat", {"Arc04.dat"}},
                {"Arc02.dat", {"Arc05.dat", "Arc05a.dat", "Arc05b.dat"}},
                {"Arc03.dat", {"Arc06.dat"}},
                {"Arc07.dat", {"Arc08.dat"}},
                {"Arc09.dat", {"Arc10.dat"}},
            },
            {0xE9, 0xEF, 0xFB},
            {4, 5, 2, 1, 1, 1, 6, 1, 1, 3, 1, 1, 1, 0, 1, 1, 1, 7, 18, 1},
            false,
        });

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Specifies plugin for decoding image files."));
}

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(8);
    const auto size_comp = read_obfuscated_number(input_file.stream);
    return size_comp == input_file.stream.left();
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto size_orig = read_obfuscated_number(input_file.stream);
    const auto size_comp = read_obfuscated_number(input_file.stream);
    const auto table_comp = input_file.stream.read(size_comp);
    const auto table_orig = algo::pack::lzss_decompress(table_comp, size_orig);
    io::MemoryByteStream table_stream(table_orig);

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->plugin = plugin_manager.get();
    const auto data_file_names = get_data_file_names(input_file, meta->plugin);
    for (const auto &data_file_name : data_file_names)
        meta->data_files.push_back(
            VirtualFileSystem::get_by_name(data_file_name));

    while (table_stream.left())
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        const auto entry_size = table_stream.read_le<u32>();
        if (entry_size != 23)
            throw err::CorruptDataError("Unexpected entry size");
        const auto file_id = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        entry->size_comp = table_stream.read_le<u32>();
        entry->offset = table_stream.read_le<u32>();
        entry->type = table_stream.read(2);
        const auto unk = table_stream.read_le<u32>(); // FFFFFFFF?
        entry->data_file_id = table_stream.read<u8>();
        if (entry->data_file_id >= meta->data_files.size()
                || !meta->data_files[entry->data_file_id])
        {
            throw err::CorruptDataError("Data file not found.");
        }
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto data_stream = meta->data_files.at(entry->data_file_id)->stream.clone();
    auto data = data_stream->seek(entry->offset).read(entry->size_comp);
    if (entry->size_orig != entry->size_comp)
        data = algo::pack::lzss_decompress(data, entry->size_orig);

    if (entry->type == "b0"_b || entry->type == "n0"_b || entry->type == "o0"_b)
    {
        io::File pseudo_file("dummy.dat", data);
        const auto image = DatImageDecoder(meta->plugin).decode(
            logger, pseudo_file);
        data = enc::png::PngImageEncoder()
            .encode(logger, image, "")
                ->stream.seek(0).read_to_eof();

    }

    if (entry->type == "j0"_b || entry->type == "k0"_b)
        decode_tinkerbell_data_headerless(data);

    if (entry->type == "u0"_b)
        decode_tinkerbell_data_with_header(data);

    auto ret = std::make_unique<io::File>(entry->path, data);
    ret->guess_extension();
    return ret;
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("cyberworks/arc");
