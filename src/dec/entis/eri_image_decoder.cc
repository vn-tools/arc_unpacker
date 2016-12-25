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

#include "dec/entis/eri_image_decoder.h"
#include <cstdlib>
#include <map>
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/entis/common/enums.h"
#include "dec/entis/common/erisa_decoder.h"
#include "dec/entis/common/gamma_decoder.h"
#include "dec/entis/common/huffman_decoder.h"
#include "dec/entis/common/sections.h"
#include "dec/entis/image/lossless.h"
#include "err.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x01\x00\x03\x00\x00\x00\x00"_b;
static const bstr magic3 = "Entis Rasterized Image"_b;

static image::EriHeader read_header(
    io::BaseByteStream &input_stream,
    const common::SectionReader &section_reader)
{
    const auto header_section = section_reader.get_section("Header");
    input_stream.seek(header_section.data_offset);
    const common::SectionReader header_section_reader(input_stream);
    const auto info_section = header_section_reader.get_section("ImageInf");
    input_stream.seek(info_section.data_offset);

    image::EriHeader header;
    header.version = input_stream.read_le<u32>();
    header.transformation = input_stream.read_le<common::Transformation>();
    header.architecture = input_stream.read_le<common::Architecture>();

    const auto tmp = input_stream.read_le<u32>();
    header.format_type  = static_cast<image::EriFormatType>(tmp & 0xFFFFFF);
    header.format_flags = static_cast<image::EriFormatFlags>(tmp >> 24);

    const s32 width         = input_stream.read_le<u32>();
    const s32 height        = input_stream.read_le<u32>();
    header.width            = std::abs(width);
    header.height           = std::abs(height);
    header.flip             = height > 0;
    header.bit_depth        = input_stream.read_le<u32>();
    header.clipped_pixel    = input_stream.read_le<u32>();
    header.sampling_flags   = input_stream.read_le<u32>();
    header.quantumized_bits = input_stream.read_le<u64>();
    header.allotted_bits    = input_stream.read_le<u64>();
    header.blocking_degree  = input_stream.read_le<u32>();
    header.lapped_block     = input_stream.read_le<u32>();
    header.frame_transform  = input_stream.read_le<u32>();
    header.frame_degree     = input_stream.read_le<u32>();

    if (header_section_reader.has_section("descript"))
    {
        const auto description_section
            = header_section_reader.get_section("descript");
        const auto description = input_stream
            .seek(description_section.data_offset)
            .read(description_section.size);
        header.description = description.substr(0, 2) == "\xFF\xFE"_b
            ? algo::utf16_to_utf8(description.substr(2)).str()
            : description.str();
    }

    return header;
}

bool EriImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic1.size()) == magic1
        && input_file.stream.read(magic2.size()) == magic2
        && input_file.stream.read(magic3.size()) == magic3;
}

static bstr decode_pixel_data(
    const image::EriHeader &header, const bstr &encoded_pixel_data)
{
    std::unique_ptr<common::BaseDecoder> decoder;

    if (header.architecture == common::Architecture::RunLengthGamma)
        decoder = std::make_unique<common::GammaDecoder>();
    else if (header.architecture == common::Architecture::RunLengthHuffman)
        decoder = std::make_unique<common::HuffmanDecoder>();
    else if (header.architecture == common::Architecture::Nemesis)
        decoder = std::make_unique<common::ErisaDecoder>();

    if (!decoder)
    {
        throw err::NotSupportedError(algo::format(
            "Architecture type %d not supported", header.architecture));
    }

    if (header.transformation != common::Transformation::Lossless)
    {
        throw err::NotSupportedError(algo::format(
            "Transformation type %d not supported", header.transformation));
    }

    decoder->set_input(encoded_pixel_data);
    return decode_lossless_pixel_data(header, *decoder);
}

res::Image EriImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x40);

    common::SectionReader section_reader(input_file.stream);
    const auto header = read_header(input_file.stream, section_reader);
    if (header.version != 0x00020100 && header.version != 0x00020200)
        throw err::UnsupportedVersionError(header.version);

    const auto stream_section = section_reader.get_section("Stream");
    input_file.stream.seek(stream_section.data_offset);
    const common::SectionReader stream_section_reader(input_file.stream);

    const auto pixel_data_sections
        = stream_section_reader.get_sections("ImageFrm");
    if (!pixel_data_sections.size())
        throw err::CorruptDataError("No pixel data found");

    std::unique_ptr<res::Image> base_image;
    if (!header.description.empty())
    {
        std::string base_name;
        const auto lines = algo::split(
            algo::replace_all(header.description, "\r", ""), '\n', false);
        logger.info("Meta:\n");
        for (const auto i : algo::range(lines.size()))
            logger.info("[line %d] %s\n", i, lines[i].c_str());
        try
        {
            std::map<std::string, std::string> meta;
            for (const auto i : algo::range(0, lines.size() & ~1, 2))
                meta[lines[i]] = lines[i + 1];
            if (meta.find("#reference-file") != meta.end())
                base_name = meta.at("#reference-file");
        }
        catch (const std::exception &e)
        {
            logger.warn("Error parsing metadata: %s\n", e.what());
        }
        if (!base_name.empty())
        {
            auto base_file = VirtualFileSystem::get_by_name(base_name);
            if (base_file && is_recognized(*base_file))
            {
                Logger dummy_logger;
                dummy_logger.mute();
                base_image = std::make_unique<res::Image>(
                    decode(dummy_logger, *base_file));
            }
        }
    }

    res::Image image(header.width, header.height * pixel_data_sections.size());
    for (const auto i : algo::range(pixel_data_sections.size()))
    {
        const auto &pixel_data_section = pixel_data_sections[i];
        const auto pixel_data = decode_pixel_data(
            header,
            input_file.stream
                .seek(pixel_data_section.data_offset)
                .read(pixel_data_section.size));

        const auto actual_depth
            = pixel_data.size() * 8 / (header.width * header.height);

        res::PixelFormat fmt;
        if (actual_depth == 32)
            fmt = res::PixelFormat::BGRA8888;
        else if (actual_depth == 24)
            fmt = res::PixelFormat::BGR888;
        else if (actual_depth == 8)
            fmt = res::PixelFormat::Gray8;
        else
            throw err::UnsupportedBitDepthError(actual_depth);

        res::Image subimage(header.width, header.height, pixel_data, fmt);
        if (header.flip)
            subimage.flip_vertically();
        image.overlay(
            subimage,
            0,
            i * header.height,
            res::Image::OverlayKind::OverwriteAll);
    }

    if (!base_image)
        return image;

    base_image->overlay(image, res::Image::OverlayKind::AddSimple);
    return *base_image;
}

static auto _ = dec::register_decoder<EriImageDecoder>("entis/eri");
