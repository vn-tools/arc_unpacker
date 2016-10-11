// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/yuzusoft/psb_image_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/kirikiri/tlg_image_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::yuzusoft;

static const bstr magic = "PSB\x00"_b;

namespace
{
    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        int x, y;
        size_t width, height;
        std::shared_ptr<res::Image> base_image;
    };

    class UnnamedDirectory final
    {
    public:
        UnnamedDirectory(
            io::BaseByteStream &input_stream);
        size_t size() const;
        io::BaseByteStream &get(const size_t index);

    private:
        uoff_t base_offset;
        std::vector<uoff_t> data_offsets;
        io::BaseByteStream &input_stream;
    };

    class NamedDirectory final
    {
    public:
        NamedDirectory(
            io::BaseByteStream &input_stream,
            const std::vector<std::string> &names);

        std::vector<std::string> get_names() const;
        bool has(const std::string &name) const;
        io::BaseByteStream &get(const std::string &name);

    private:
        uoff_t base_offset;
        std::vector<size_t> name_indices;
        std::vector<uoff_t> data_offsets;
        io::BaseByteStream &input_stream;
        const std::vector<std::string> &names;
    };

    struct BasicInfo final
    {
        uoff_t offset_names;
        uoff_t offset_strings;
        uoff_t offset_strings_data;
        uoff_t offset_chunk_offsets;
        uoff_t offset_chunk_sizes;
        uoff_t offset_chunk_data;
        uoff_t offset_directory;
    };
}

static std::shared_ptr<res::Image> read_image(
    const Logger &logger,
    const CustomArchiveEntry &entry,
    io::BaseByteStream &input_stream)
{
    io::File pseudo_file(
        "dummy.dat",
        input_stream.seek(entry.offset).read(entry.size));

    if (entry.path.has_extension("tlg"))
    {
        return std::make_shared<res::Image>(
            dec::kirikiri::TlgImageDecoder().decode(logger, pseudo_file));
    }

    if (entry.path.has_extension("png"))
    {
        return std::make_shared<res::Image>(
            dec::png::PngImageDecoder().decode(logger, pseudo_file));
    }

    throw std::logic_error("Unknown image extension");
}

static BasicInfo read_basic_info(io::BaseByteStream &input_stream)
{
    BasicInfo ret;
    ret.offset_names = input_stream.read_le<u32>();
    ret.offset_strings = input_stream.read_le<u32>();
    ret.offset_strings_data = input_stream.read_le<u32>();
    ret.offset_chunk_offsets = input_stream.read_le<u32>();
    ret.offset_chunk_sizes = input_stream.read_le<u32>();
    ret.offset_chunk_data = input_stream.read_le<u32>();
    ret.offset_directory = input_stream.read_le<u32>();
    return ret;
}

static unsigned long read_variable_integer(
    io::BaseByteStream &input_stream, const size_t bytes)
{
    size_t ret = 0;
    for (const auto i : algo::range(bytes))
        ret |= input_stream.read<u8>() << (i * 8);
    return ret;
}

static double read_number(io::BaseByteStream &input_stream)
{
    static const std::vector<unsigned long> type_to_kind =
    {
        0, 1, 2, 2, 3, 3, 3, 3, 3, 4, 4,
        4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7,
        7, 7, 7, 8, 8, 8, 8, 9, 9, 10, 11, 12
    };

    const auto type = input_stream.read<u8>();
    const auto kind = type_to_kind.at(type);

    if (kind == 1) return 0;
    if (kind == 2) return 1;
    if (kind == 3) return read_variable_integer(input_stream, type - 4);
    if (kind == 9) return input_stream.read_le<f32>();
    if (kind == 10) return input_stream.read_le<f64>();
    throw err::NotSupportedError("Unknown number type");
}

static std::string read_string(
    io::BaseByteStream &input_stream,
    const std::vector<unsigned long> &string_offsets,
    const uoff_t offset_strings_data)
{
    const auto n = input_stream.read<u8>() - 0x14;
    const auto string_idx = read_variable_integer(input_stream, n);
    const auto string_offset
        = string_offsets.at(string_idx) + offset_strings_data;
    return input_stream.seek(string_offset).read_to_zero().str(true);
}

static std::vector<unsigned long> read_array(io::BaseByteStream &input_stream)
{
    const auto n = input_stream.read<u8>() - 0xC;
    const auto entry_count = read_variable_integer(input_stream, n);
    const auto entry_size = input_stream.read<u8>() - 0xC;
    std::vector<unsigned long> ret;
    for (const auto i : algo::range(entry_count))
        ret.push_back(read_variable_integer(input_stream, entry_size));
    return ret;
}

static std::vector<std::string> read_names(
    io::BaseByteStream &input_stream, const BasicInfo &basic_info)
{
    input_stream.seek(basic_info.offset_names);
    const auto str1 = read_array(input_stream);
    const auto str2 = read_array(input_stream);
    const auto str3 = read_array(input_stream);
    std::vector<std::string> ret;
    for (const auto i : algo::range(str3.size()))
    {
        std::string str;
        const auto a = str3.at(i);
        auto b = str2.at(a);
        while (true)
        {
            const auto c = str2.at(b);
            const auto d = str1.at(c);
            const auto e = b - d;
            b = c;
            str = static_cast<char>(e) + str;
            if (!b)
                break;
        }
        ret.push_back(str);
    }
    return ret;
}

UnnamedDirectory::UnnamedDirectory(io::BaseByteStream &input_stream)
    : input_stream(input_stream)
{
    if (input_stream.read<u8>() != 0x20)
        throw err::CorruptDataError("Unexpected marker");
    for (const auto x : read_array(input_stream))
        data_offsets.push_back(x);
    base_offset = input_stream.pos();
}

size_t UnnamedDirectory::size() const
{
    return data_offsets.size();
}

io::BaseByteStream &UnnamedDirectory::get(const size_t index)
{
    return input_stream.seek(base_offset + data_offsets.at(index));
}

NamedDirectory::NamedDirectory(
    io::BaseByteStream &input_stream, const std::vector<std::string> &names)
    : input_stream(input_stream), names(names)
{
    if (input_stream.read<u8>() != 0x21)
        throw err::CorruptDataError("Unexpected marker");
    for (const auto x : read_array(input_stream))
        name_indices.push_back(x);
    for (const auto x : read_array(input_stream))
        data_offsets.push_back(x);
    base_offset = input_stream.pos();
}

std::vector<std::string> NamedDirectory::get_names() const
{
    std::vector<std::string> ret;
    for (const auto x : name_indices)
        ret.push_back(names.at(x));
    return ret;
}

bool NamedDirectory::has(const std::string &name) const
{
    for (const auto i : algo::range(name_indices.size()))
        if (names.at(name_indices[i]) == name)
            return true;
    return false;
}

io::BaseByteStream &NamedDirectory::get(const std::string &name)
{
    for (const auto i : algo::range(name_indices.size()))
        if (names.at(name_indices[i]) == name)
            return input_stream.seek(base_offset + data_offsets.at(i));
    throw err::CorruptDataError("Missing entry '" + name + "'");
}

algo::NamingStrategy PsbImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

bool PsbImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PsbImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto type = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);

    const auto basic_info = read_basic_info(input_file.stream);
    const auto names = read_names(input_file.stream, basic_info);
    const auto string_offsets
        = read_array(input_file.stream.seek(basic_info.offset_strings));
    const auto chunk_offsets
        = read_array(input_file.stream.seek(basic_info.offset_chunk_offsets));
    const auto chunk_sizes
        = read_array(input_file.stream.seek(basic_info.offset_chunk_sizes));

    input_file.stream.seek(basic_info.offset_directory);
    NamedDirectory root_directory(input_file.stream, names);

    const auto width = read_number(root_directory.get("width"));
    const auto height = read_number(root_directory.get("height"));
    auto meta = std::make_unique<dec::ArchiveMeta>();

    for (const auto &name : root_directory.get_names())
    {
        const auto is_tlg = name.find(".tlg") != std::string::npos;
        const auto is_png = name.find(".png") != std::string::npos;

        if (is_tlg || is_png)
        {
            auto &data_stream = root_directory.get(name);
            const auto n = data_stream.read<u8>() - 0x18;
            const auto chunk_index = read_variable_integer(data_stream, n);

            auto entry = std::make_unique<CustomArchiveEntry>();
            entry->path = input_file.path.stem() + "_" + name;
            entry->offset = basic_info.offset_chunk_data
                + chunk_offsets.at(chunk_index);
            entry->size = chunk_sizes.at(chunk_index);
            meta->entries.push_back(std::move(entry));
        }
        else if (name != "width" && name != "height" && name != "layers")
        {
            logger.warn("Unknown entry: %s\n", name.c_str());
        }
    }

    if (root_directory.has("layers"))
    {
        UnnamedDirectory layers_directory(root_directory.get("layers"));
        CustomArchiveEntry *chosen_entry = nullptr;
        for (const auto i : algo::range(layers_directory.size()))
        {
            NamedDirectory layer_directory(layers_directory.get(i), names);

            int layer_id = read_number(layer_directory.get("layer_id"));
            auto perhaps_name = algo::format("%d", layer_id);

            for (const auto j : algo::range(meta->entries.size()))
            {
                auto entry = static_cast<CustomArchiveEntry*>(
                    meta->entries[j].get());
                if (entry->path.str().find(perhaps_name) != std::string::npos)
                    chosen_entry = entry;
            }

            for (const auto &name : layer_directory.get_names())
            {
                if (name == "name")
                {
                    logger.info(
                        "%s: %s\n",
                        name.c_str(),
                        read_string(
                            layer_directory.get(name),
                            string_offsets,
                            basic_info.offset_strings_data).c_str());
                }
                else
                {
                    logger.info(
                        "%s: %f\n",
                        name.c_str(),
                        read_number(layer_directory.get(name)));
                }
            }
            logger.info("\n");

            if (!chosen_entry)
                throw err::CorruptDataError("Unknown entry");

            chosen_entry->x = read_number(layer_directory.get("left"));
            chosen_entry->y = read_number(layer_directory.get("top"));
            chosen_entry->width = read_number(layer_directory.get("width"));
            chosen_entry->height = read_number(layer_directory.get("height"));
            chosen_entry->path.change_stem(
                chosen_entry->path.stem() + "_" + read_string(
                    layer_directory.get("name"),
                    string_offsets,
                    basic_info.offset_strings_data));
        }

        auto base_image = read_image(logger, *chosen_entry, input_file.stream);
        for (const auto &entry : meta->entries)
        {
            if (entry.get() != chosen_entry)
            {
                static_cast<CustomArchiveEntry*>(entry.get())->base_image
                    = base_image;
            }
        }
    }

    return meta;
}

std::unique_ptr<io::File> PsbImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto image = read_image(logger, *entry, input_file.stream);

    if (entry->base_image)
    {
        auto base_image = std::make_unique<res::Image>(*entry->base_image);
        base_image->overlay(
            *image,
            entry->x,
            entry->y,
            res::Image::OverlayKind::OverwriteNonTransparent);
        image = std::move(base_image);
    }

    return enc::png::PngImageEncoder().encode(logger, *image, entry->path);
}

static auto _ = dec::register_decoder<PsbImageArchiveDecoder>("yuzusoft/psb");
