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

#include "dec/malie/dzi_image_archive_decoder.h"
#include <regex>
#include <utility>
#include "algo/range.h"
#include "dec/google/webp_image_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::malie;

static const auto magic = "DZI\r\n"_b;
static const auto pair_re = std::regex("^(\\d+),(\\d+)");

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        size_t width;
        size_t height;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        size_t horizontal_block_count;
        size_t vertical_block_count;
        std::vector<std::string> texture_paths;
    };
}

static std::vector<std::string> split(
    const std::string &input, const char delimiter)
{
    std::vector<std::string> output;
    size_t start = 0, end = 0;
    while ((end = input.find(delimiter, start)) != std::string::npos)
    {
        output.push_back(input.substr(start, end - start));
        start = end + 1;
    }
    output.push_back(input.substr(start));
    return output;
}

static res::Image get_texture_image(
    const Logger &logger,
    const io::path &input_file_path,
    const std::string &texture_name)
{
    std::unique_ptr<io::File> texture_file;
    auto texture_path = input_file_path.parent() / "tex" / texture_name;

    texture_path.change_extension("webp");
    texture_file = VirtualFileSystem::get_by_path(texture_path.str());
    if (texture_file)
        return dec::google::WebpImageDecoder().decode(logger, *texture_file);

    texture_path.change_extension("png");
    texture_file = VirtualFileSystem::get_by_path(texture_path.str());
    if (texture_file)
        return dec::png::PngImageDecoder().decode(logger, *texture_file);

    throw err::FileNotFoundError("Block texture not found");
}

bool DziImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

algo::NamingStrategy DziImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

std::unique_ptr<dec::ArchiveMeta> DziImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read_line() != "DZI"_b)
        throw err::CorruptDataError("Invalid file header");

    std::smatch match;

    auto line = input_file.stream.read_line().str();
    if (!std::regex_match(line, match, pair_re))
        throw err::CorruptDataError("Invalid image size");

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->width = std::stoi(match[1]);
    meta->height = std::stoi(match[2]);
    if (!meta->width || !meta->height)
        throw err::CorruptDataError("Invalid image size");

    const auto file_count = std::stoi(input_file.stream.read_line().str());
    for (const auto i : algo::range(file_count))
    {
        line = input_file.stream.read_line().str();
        if (!std::regex_match(line, match, pair_re))
            throw err::CorruptDataError("Invalid block size");

        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->horizontal_block_count = std::stoi(match[1]);
        entry->vertical_block_count = std::stoi(match[2]);

        for (const auto y : algo::range(entry->vertical_block_count))
        {
            line = input_file.stream.read_line().str();
            const auto file_names = split(line, ',');
            for (const auto x : algo::range(entry->horizontal_block_count))
                entry->texture_paths.push_back(file_names.at(x));
        }

        meta->entries.push_back(std::move(entry));
    }

    return std::move(meta);
}

std::unique_ptr<io::File> DziImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    res::Image image(meta->width, meta->height);
    for (const auto y : algo::range(entry->vertical_block_count))
    for (const auto x : algo::range(entry->horizontal_block_count))
    {
        const auto texture_name
            = entry->texture_paths.at(y * entry->horizontal_block_count + x);

        if (texture_name.empty())
            continue;

        image.overlay(
            get_texture_image(logger, input_file.path, texture_name),
            x * 256,
            y * 256,
            res::Image::OverlayKind::OverwriteAll);
    }
    return enc::png::PngImageEncoder().encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<DziImageArchiveDecoder>("malie/dzi");
