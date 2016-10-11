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

#include "dec/nscripter/nsa_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/nscripter/nsa_encrypted_stream.h"
#include "dec/nscripter/spb_image_decoder.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::nscripter;

namespace
{
    enum class CompressionType : u8
    {
        None    = 0,
        Spb     = 1,
        Lzss    = 2,
    };

    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        CompressionType compression_type;
    };
}

NsaArchiveDecoder::NsaArchiveDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--nsa-key"})
                ->set_value_name("KEY")
                ->set_description("Decryption key");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_switch("nsa-key"))
                key = arg_parser.get_switch("nsa-key");
        });
}

bool NsaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("nsa")
        || input_file.path.has_extension("dat");
}

std::unique_ptr<dec::ArchiveMeta> NsaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    NsaEncryptedStream input_stream(input_file.stream, key);
    input_stream.seek(0);

    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = input_stream.read_be<u16>();
    const auto offset_to_data = input_stream.read_be<u32>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = input_stream.read_to_zero().str(true);
        entry->compression_type = input_stream.read<CompressionType>();
        entry->offset = input_stream.read_be<u32>() + offset_to_data;
        entry->size_comp = input_stream.read_be<u32>();
        entry->size_orig = input_stream.read_be<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> NsaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    NsaEncryptedStream input_stream(input_file.stream, key);

    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    const auto data = input_stream
        .seek(entry->offset)
        .read(entry->size_comp);

    if (entry->compression_type == CompressionType::None)
        return std::make_unique<io::File>(entry->path, data);

    if (entry->compression_type == CompressionType::Lzss)
    {
        algo::pack::BitwiseLzssSettings settings;
        settings.position_bits = 8;
        settings.size_bits = 4;
        settings.min_match_size = 2;
        settings.initial_dictionary_pos = 239;
        return std::make_unique<io::File>(
            entry->path,
            algo::pack::lzss_decompress(data, entry->size_orig, settings));
    }

    if (entry->compression_type == CompressionType::Spb)
    {
        const auto decoder = SpbImageDecoder();
        const auto encoder = enc::png::PngImageEncoder();
        io::File spb_file("dummy.bmp", data);
        return encoder.encode(
            logger, decoder.decode(logger, spb_file), entry->path);
    }

    throw err::NotSupportedError("Unknown compression type");
}

static auto _ = dec::register_decoder<NsaArchiveDecoder>("nscripter/nsa");
