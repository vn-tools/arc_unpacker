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

#include "dec/valkyria/odn_archive_decoder.h"
#include <map>
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "io/xor_byte_stream.h"

using namespace au;
using namespace au::dec::valkyria;

namespace
{
    enum class OdnVariant : u8
    {
        Variant1,
        Variant1XOR,
        Variant2,
        Variant3,
    };

    struct OdnArchiveMeta final : dec::ArchiveMeta
    {
        OdnVariant variant;
    };
}

// this is pathetic but their games actually hardcode this
static const std::vector<std::tuple<size_t, size_t>> known_image_sizes =
{
    std::make_tuple(1024, 768),
    std::make_tuple(400, 200),
    std::make_tuple(800, 600),
    std::make_tuple(640, 480),
};

static size_t hex_to_int(const bstr &input)
{
    size_t ret = 0;
    for (const auto c : input)
    {
        ret *= 16;
        if (c >= 'a' && c <= 'f') ret += c + 10 - 'a';
        if (c >= 'A' && c <= 'F') ret += c + 10 - 'A';
        if (c >= '0' && c <= '9') ret += c - '0';
    }
    return ret;
}

bool OdnArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("odn");
}

static OdnVariant guess_odn_variant(io::BaseByteStream &input_stream)
{
    const auto likely_file1_offset = input_stream.seek(8).read(8);
    if (likely_file1_offset == "00000000"_b)
        return OdnVariant::Variant1;
    else if (likely_file1_offset == "\xc7\xc6\xc5\xc4\xc3\xc2\xc1\xc0"_b)
        return OdnVariant::Variant1XOR;

    const auto likely_file1_prefix = input_stream.seek(0).read(4);
    const auto maybe_file2_prefix1 = input_stream.seek(16).read(4);
    const auto maybe_file2_prefix2 = input_stream.seek(24).read(4);

    if (likely_file1_prefix == maybe_file2_prefix1)
        return OdnVariant::Variant2;

    if (likely_file1_prefix == maybe_file2_prefix2)
        return OdnVariant::Variant3;

    throw err::RecognitionError("Not an ODN archive");
}

static void fill_sizes(
    const io::BaseByteStream &input_stream,
    std::vector<std::unique_ptr<dec::ArchiveEntry>> &entries)
{
    if (!entries.size())
        return;
    for (const auto i : algo::range(1, entries.size()))
    {
        auto prev_entry
            = static_cast<dec::PlainArchiveEntry*>(entries[i - 1].get());
        auto current_entry
            = static_cast<dec::PlainArchiveEntry*>(entries[i].get());
        prev_entry->size = current_entry->offset - prev_entry->offset;
    }
    auto last_entry = static_cast<dec::PlainArchiveEntry*>(
        entries.back().get());
    last_entry->size = input_stream.size() - last_entry->offset;
}

static void read_meta_v1(
    dec::ArchiveMeta *meta,
    io::BaseByteStream &input_stream)
{
    while (true)
    {
        auto entry = std::make_unique<dec::PlainArchiveEntry>();
        entry->path = input_stream.read(8).str();
        entry->offset = hex_to_int(input_stream.read(8));
        if (entry->path == "END_ffff" || entry->path == "ffffffff")
            break;
        meta->entries.push_back(std::move(entry));
    }
    for (auto &e : meta->entries)
    {
        auto entry = static_cast<dec::PlainArchiveEntry*>(e.get());
        entry->offset += input_stream.pos();
    }
    fill_sizes(input_stream, meta->entries);
}

static void read_meta_v2(
    dec::ArchiveMeta* meta,
    io::BaseByteStream &input_stream)
{
    const auto data_pos = hex_to_int(input_stream.seek(8).read(8));
    input_stream.seek(0);
    while (input_stream.pos() < data_pos)
    {
        auto entry = std::make_unique<dec::PlainArchiveEntry>();
        entry->path = input_stream.read(8).str();
        entry->offset = hex_to_int(input_stream.read(8));
        meta->entries.push_back(std::move(entry));
    }
    fill_sizes(input_stream, meta->entries);
}

static void read_meta_v3(
    dec::ArchiveMeta* meta,
    io::BaseByteStream &input_stream)
{
    const auto data_pos = hex_to_int(input_stream.seek(8).read(8));
    input_stream.seek(0);
    while (input_stream.pos() < data_pos)
    {
        auto entry = std::make_unique<dec::PlainArchiveEntry>();
        entry->path = input_stream.read(8).str();
        entry->offset = hex_to_int(input_stream.read(8));
        input_stream.skip(8);
        meta->entries.push_back(std::move(entry));
    }
    fill_sizes(input_stream, meta->entries);
}

static bstr decompress(const bstr &input, const size_t chunk_size)
{
    bstr output;
    io::MemoryByteStream input_stream(input);
    while (input_stream.left())
    {
        const u8 flags = input_stream.read<u8>();
        u16 mask = 1;
        do
        {
            if (!input_stream.left())
                break;
            const bstr chunk = input_stream.read(chunk_size);
            const auto repetitions = (flags & mask)
                ? input_stream.read_be<u16>() + 1
                : 1;
            for (const auto i : algo::range(repetitions))
                output += chunk;
            mask <<= 1;
        }
        while (mask <= 128);
    }
    return output;
}

static bstr decompress_bgr(const Logger &logger, const bstr &input)
{
    const auto output = decompress(input, 3);
    const auto encoder = enc::png::PngImageEncoder();
    for (const auto &known_image_size : known_image_sizes)
    {
        const auto width = std::get<0>(known_image_size);
        const auto height = std::get<1>(known_image_size);
        if (output.size() == width * height * 3)
        {
            res::Image image(width, height, output, res::PixelFormat::BGR888);
            image.flip_vertically();
            return encoder.encode(logger, image, "dummy.png")
                ->stream.seek(0).read_to_eof();
        }
    }
    return output;
}

static bstr decompress_bgra(const Logger &logger, const bstr &input)
{
    const auto output = decompress(input, 4);
    const auto encoder = enc::png::PngImageEncoder();
    for (const auto &known_image_size : known_image_sizes)
    {
        const auto width = std::get<0>(known_image_size);
        const auto height = std::get<1>(known_image_size);
        if (output.size() == width * height * 4)
        {
            res::Image image(width, height, output, res::PixelFormat::BGRA8888);
            image.flip_vertically();
            return encoder.encode(logger, image, "dummy.png")
                ->stream.seek(0).read_to_eof();
        }
    }
    return output;
}

std::unique_ptr<dec::ArchiveMeta> OdnArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<OdnArchiveMeta>();
    meta->variant = guess_odn_variant(input_file.stream);
    switch (meta->variant)
    {
        case OdnVariant::Variant1:
            read_meta_v1(meta.get(), input_file.stream);
            break;
        case OdnVariant::Variant1XOR:
            read_meta_v1(meta.get(), io::XORByteStream(input_file.stream));
            break;
        case OdnVariant::Variant2:
            read_meta_v2(meta.get(), input_file.stream);
            break;
        case OdnVariant::Variant3:
            read_meta_v3(meta.get(), input_file.stream);
            break;
        default:
            throw std::logic_error("Invalid archive type");
    }
    return meta;
}

std::unique_ptr<io::File> OdnArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const OdnArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto prefix = entry->path.str().substr(0, 4);
    io::XORByteStream xorstream(input_file.stream);
    io::BaseByteStream& stream = meta->variant == OdnVariant::Variant1XOR ? xorstream : input_file.stream;
    auto data = stream.seek(entry->offset).read(entry->size);
    if (prefix == "back")
        data = decompress_bgr(logger, data);
    if (prefix == "codn" || prefix == "cccc")
        data = decompress_bgra(logger, data);
    auto ret = std::make_unique<io::File>(entry->path, data);
    ret->guess_extension();
    return ret;
}

static auto _ = dec::register_decoder<OdnArchiveDecoder>("valkyria/odn");
