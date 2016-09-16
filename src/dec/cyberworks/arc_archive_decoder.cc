#include "dec/cyberworks/arc_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/ptr.h"
#include "algo/range.h"
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
        ArcArchivePlugin plugin;
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
    const io::File &input_file, const ArcArchivePlugin &plugin)
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

static std::map<ImageParameter, u32> read_image_parameters(
    io::BaseByteStream &input_stream, const ArcArchivePlugin &plugin)
{
    std::map<ImageParameter, u32> parameters;
    u8 step[3] = {0};
    for (const auto param : plugin.img_param_order)
    {
        step[0] = input_stream.read<u8>();
        if (step[0] == plugin.img_delim[2])
            step[0] = 0;
        u8 tmp = input_stream.read<u8>();
        while (tmp != plugin.img_delim[1])
        {
            if (tmp == plugin.img_delim[0])
                step[2]++;
            else if (tmp == plugin.img_delim[2])
                step[1] = 0;
            else
                step[1] = tmp;
            tmp = input_stream.read<u8>();
        }
        const u32 value
            = step[2] * plugin.img_delim[0] * plugin.img_delim[0]
            + step[1] * plugin.img_delim[0]
            + step[0];
        step[1] = 0;
        step[2] = 0;
        parameters[param] = value;
    }
    return parameters;
}

static void decode_picture(
    const Logger &logger, const ArcArchivePlugin &plugin, bstr &data)
{
    if (!data.size())
        return;

    if (data[0] == 'a')
    {
        io::MemoryByteStream data_stream(data);
        data_stream.seek(2);

        const auto parameters = read_image_parameters(data_stream, plugin);
        const auto width = parameters.at(ImageParameter::Width);
        const auto height = parameters.at(ImageParameter::Height);
        const auto bitmap_size = parameters.at(ImageParameter::BitmapSize);
        const auto channels = bitmap_size / (width * height);
        const auto stride = ((channels * width) + 3) & ~3;
        const auto data_offset = data_stream.pos();

        res::PixelFormat format;
        if (channels == 3)
            format = res::PixelFormat::BGR888;
        else if (channels == 1)
            format = res::PixelFormat::Gray8;
        else
            throw err::UnsupportedChannelCountError(channels);

        res::Image image(width, height);
        for (const auto y : algo::range(height))
        {
            data_stream.seek(data_offset + stride * y);
            res::Image row(width, 1, data_stream, format);
            for (const auto x : algo::range(width))
                image.at(x, y) = row.at(x, 0);
        }

        const auto encoder = enc::png::PngImageEncoder();
        data = encoder.encode(logger, image, "")->stream.seek(0).read_to_eof();
        return;
    }

    if (data[0] == 'c')
    {
        // a PNG file
        data = data.substr(5);
        return;
    }
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
            {
                ImageParameter::BitmapSize,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Width,
                ImageParameter::Height,
                ImageParameter::Unknown,
            },
            {0xE9, 0xEF, 0xFB},
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
            {
                ImageParameter::Width,
                ImageParameter::BitmapSize,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Height,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Type,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
                ImageParameter::Unknown,
            },
            {0xE9, 0xEF, 0xFB},
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
        decode_picture(logger, meta->plugin, data);

    if (entry->type == "j0"_b || entry->type == "k0"_b)
        decode_tinkerbell_data_headerless(data);

    if (entry->type == "u0"_b)
        decode_tinkerbell_data_with_header(data);

    auto ret = std::make_unique<io::File>(entry->path, data);
    ret->guess_extension();
    return ret;
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("cyberworks/arc");
