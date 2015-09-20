#include "fmt/entis/eri_converter.h"
#include "err.h"
#include "fmt/entis/common/enums.h"
#include "fmt/entis/common/gamma_decoder.h"
#include "fmt/entis/common/huffman_decoder.h"
#include "fmt/entis/common/nemesis_decoder.h"
#include "fmt/entis/common/sections.h"
#include "fmt/entis/image/lossless.h"
#include "util/image.h"
#include "util/range.h"
#include "util/format.h"

using namespace au;
using namespace au::fmt::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x01\x00\x03\x00\x00\x00\x00"_b;
static const bstr magic3 = "Entis Rasterized Image"_b;

static image::EriHeader read_header(
    io::IO &io, common::SectionReader &section_reader)
{
    auto header_section = section_reader.get_section("Header");
    io.seek(header_section.offset);
    common::SectionReader header_section_reader(io);
    header_section = header_section_reader.get_section("ImageInf");
    io.seek(header_section.offset);

    image::EriHeader header;
    header.version = io.read_u32_le();
    header.transformation
        = static_cast<common::Transformation>(io.read_u32_le());
    header.architecture = static_cast<common::Architecture>(io.read_u32_le());

    header.format_type      = io.read_u32_le();
    s32 width               = io.read_u32_le();
    s32 height              = io.read_u32_le();
    header.width            = std::abs(width);
    header.height           = std::abs(height);
    header.flip             = height > 0;
    header.bit_depth        = io.read_u32_le();
    header.clipped_pixel    = io.read_u32_le();
    header.sampling_flags   = io.read_u32_le();
    header.quantumized_bits = io.read_u64_le();
    header.allotted_bits    = io.read_u64_le();
    header.blocking_degree  = io.read_u32_le();
    header.lapped_block     = io.read_u32_le();
    header.frame_transform  = io.read_u32_le();
    header.frame_degree     = io.read_u32_le();
    return header;
}

bool EriConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic1.size()) == magic1
        && file.io.read(magic2.size()) == magic2
        && file.io.read(magic3.size()) == magic3;
}

static bstr decode_pixel_data(
    const image::EriHeader &header, const bstr &encoded_pixel_data)
{
    std::unique_ptr<common::Decoder> decoder;
    if (header.architecture == common::Architecture::RunLengthGamma)
        decoder.reset(new common::GammaDecoder());
    else if (header.architecture == common::Architecture::RunLengthHuffman)
        decoder.reset(new common::HuffmanDecoder());
    else if (header.architecture == common::Architecture::Nemesis)
        decoder.reset(new common::NemesisDecoder());
    else
    {
        throw err::NotSupportedError(util::format(
            "Architecture type %d not supported", header.architecture));
    }

    if (header.transformation != common::Transformation::Lossless)
    {
        throw err::NotSupportedError(util::format(
            "Transformation type %d not supported", header.transformation));
    }

    decoder->set_input(encoded_pixel_data);
    return decode_lossless_pixel_data(header, *decoder);
}

std::unique_ptr<File> EriConverter::decode_internal(File &file) const
{
    file.io.seek(0x40);

    common::SectionReader section_reader(file.io);
    auto header = read_header(file.io, section_reader);
    if (header.version != 0x00020100 && header.version != 0x00020200)
        throw err::UnsupportedVersionError(header.version);

    auto stream_section = section_reader.get_section("Stream");
    file.io.seek(stream_section.offset);
    common::SectionReader stream_section_reader(file.io);

    auto pixel_data_sections = stream_section_reader.get_sections("ImageFrm");
    if (!pixel_data_sections.size())
        throw err::CorruptDataError("No pixel data found");

    pix::Grid pixels(header.width, header.height * pixel_data_sections.size());

    for (auto i : util::range(pixel_data_sections.size()))
    {
        auto &pixel_data_section = pixel_data_sections[i];
        file.io.seek(pixel_data_section.offset);
        auto encoded_pixel_data = file.io.read(pixel_data_section.size);
        auto decoded_pixel_data = decode_pixel_data(header, encoded_pixel_data);

        pix::Format fmt;
        if (header.bit_depth == 32)
            fmt = pix::Format::BGRA8888;
        else if (header.bit_depth == 24)
            fmt = pix::Format::BGR888;
        else if (header.bit_depth == 8)
            fmt = pix::Format::Gray8;
        else
            throw err::UnsupportedBitDepthError(header.bit_depth);

        pix::Grid subimage_pixels(
            header.width, header.height, decoded_pixel_data, fmt);
        if (header.flip)
            subimage_pixels.flip();
        for (auto y : util::range(header.height))
            for (auto x : util::range(header.width))
                pixels.at(x, y + i * header.height) = subimage_pixels.at(x, y);
    }

    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<EriConverter>("entis/eri");
