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

#include "dec/tactics/arc_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "algo/str.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::tactics;

static const bstr magic = "TACTICS_ARC_FILE"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bstr key;
        CompressionMethod compression_method;
    };
}

static bstr custom_lzss_decompress(const bstr &input, const size_t size_orig)
{
    static const uint16_t backref_table[] = {
        0x0001, 0x0804, 0x1001, 0x2001, 0x0002, 0x0805, 0x1002, 0x2002,
        0x0003, 0x0806, 0x1003, 0x2003, 0x0004, 0x0807, 0x1004, 0x2004,
        0x0005, 0x0808, 0x1005, 0x2005, 0x0006, 0x0809, 0x1006, 0x2006,
        0x0007, 0x080A, 0x1007, 0x2007, 0x0008, 0x080B, 0x1008, 0x2008,
        0x0009, 0x0904, 0x1009, 0x2009, 0x000A, 0x0905, 0x100A, 0x200A,
        0x000B, 0x0906, 0x100B, 0x200B, 0x000C, 0x0907, 0x100C, 0x200C,
        0x000D, 0x0908, 0x100D, 0x200D, 0x000E, 0x0909, 0x100E, 0x200E,
        0x000F, 0x090A, 0x100F, 0x200F, 0x0010, 0x090B, 0x1010, 0x2010,
        0x0011, 0x0A04, 0x1011, 0x2011, 0x0012, 0x0A05, 0x1012, 0x2012,
        0x0013, 0x0A06, 0x1013, 0x2013, 0x0014, 0x0A07, 0x1014, 0x2014,
        0x0015, 0x0A08, 0x1015, 0x2015, 0x0016, 0x0A09, 0x1016, 0x2016,
        0x0017, 0x0A0A, 0x1017, 0x2017, 0x0018, 0x0A0B, 0x1018, 0x2018,
        0x0019, 0x0B04, 0x1019, 0x2019, 0x001A, 0x0B05, 0x101A, 0x201A,
        0x001B, 0x0B06, 0x101B, 0x201B, 0x001C, 0x0B07, 0x101C, 0x201C,
        0x001D, 0x0B08, 0x101D, 0x201D, 0x001E, 0x0B09, 0x101E, 0x201E,
        0x001F, 0x0B0A, 0x101F, 0x201F, 0x0020, 0x0B0B, 0x1020, 0x2020,
        0x0021, 0x0C04, 0x1021, 0x2021, 0x0022, 0x0C05, 0x1022, 0x2022,
        0x0023, 0x0C06, 0x1023, 0x2023, 0x0024, 0x0C07, 0x1024, 0x2024,
        0x0025, 0x0C08, 0x1025, 0x2025, 0x0026, 0x0C09, 0x1026, 0x2026,
        0x0027, 0x0C0A, 0x1027, 0x2027, 0x0028, 0x0C0B, 0x1028, 0x2028,
        0x0029, 0x0D04, 0x1029, 0x2029, 0x002A, 0x0D05, 0x102A, 0x202A,
        0x002B, 0x0D06, 0x102B, 0x202B, 0x002C, 0x0D07, 0x102C, 0x202C,
        0x002D, 0x0D08, 0x102D, 0x202D, 0x002E, 0x0D09, 0x102E, 0x202E,
        0x002F, 0x0D0A, 0x102F, 0x202F, 0x0030, 0x0D0B, 0x1030, 0x2030,
        0x0031, 0x0E04, 0x1031, 0x2031, 0x0032, 0x0E05, 0x1032, 0x2032,
        0x0033, 0x0E06, 0x1033, 0x2033, 0x0034, 0x0E07, 0x1034, 0x2034,
        0x0035, 0x0E08, 0x1035, 0x2035, 0x0036, 0x0E09, 0x1036, 0x2036,
        0x0037, 0x0E0A, 0x1037, 0x2037, 0x0038, 0x0E0B, 0x1038, 0x2038,
        0x0039, 0x0F04, 0x1039, 0x2039, 0x003A, 0x0F05, 0x103A, 0x203A,
        0x003B, 0x0F06, 0x103B, 0x203B, 0x003C, 0x0F07, 0x103C, 0x203C,
        0x0801, 0x0F08, 0x103D, 0x203D, 0x1001, 0x0F09, 0x103E, 0x203E,
        0x1801, 0x0F0A, 0x103F, 0x203F, 0x2001, 0x0F0B, 0x1040, 0x2040,
    };

    auto input_ptr = algo::make_ptr(input);

    size_t size_orig2 = 0;
    while (input_ptr.left())
    {
        const auto i = input_ptr.pos();
        const auto c = *input_ptr++;
        size_orig2 |= (c & 0x7F) << (i * 7);
        if (!(c & 0x80))
            break;
    }

    bstr output(size_orig);
    auto output_ptr = algo::make_ptr(output);
    while (output_ptr.left())
    {
        const auto c = *input_ptr++;
        if (c & 3)
        {
            size_t look_behind = 0;
            for (const auto i : algo::range(backref_table[c] >> 11))
                look_behind |= *input_ptr++ << (i * 8);
            look_behind += backref_table[c] & 0x700;
            const auto size = backref_table[c] & 0xFF;
            output_ptr.append_self(-look_behind, size);
        }
        else
        {
            auto size = (c >> 2) + 1;
            if (size >= 0x3D)
            {
                const auto tmp = size - 0x3C;
                size = 0;
                for (const auto i : algo::range(tmp))
                    size |= *input_ptr++ << (i * 8);
                size++;
            }
            output_ptr.append_from(input_ptr, size);
        }
    }
    return output;
}

static std::unique_ptr<CustomArchiveMeta> read_meta_v0(io::File &input_file)
{
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>();
    if (size_comp > 1024 * 1024 * 10)
        throw err::BadDataSizeError();

    input_file.stream.skip(4);
    const auto table_data
        = algo::unxor(input_file.stream.read(size_comp), 0xFF);
    io::MemoryByteStream table_stream(
        algo::pack::lzss_decompress(table_data, size_orig));

    const auto data_start = input_file.stream.pos();
    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->key = table_stream.read_to_zero();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<dec::CompressedArchiveEntry>();
        entry->offset = table_stream.read_le<u32>() + data_start;
        entry->size_comp = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        auto name_size = table_stream.read_le<u32>();

        table_stream.skip(8);
        entry->path = algo::sjis_to_utf8(table_stream.read(name_size)).str();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

static std::unique_ptr<CustomArchiveMeta> read_meta_v1(io::File &input_file)
{
    auto meta = std::make_unique<CustomArchiveMeta>();
    while (input_file.stream.left())
    {
        auto entry = std::make_unique<dec::CompressedArchiveEntry>();
        entry->size_comp = input_file.stream.read_le<u32>();
        if (!entry->size_comp)
            break;
        entry->size_orig = input_file.stream.read_le<u32>();
        const auto name_size = input_file.stream.read_le<u32>();
        input_file.stream.skip(8);
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read(name_size)).str();
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(entry->size_comp);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

ArcArchiveDecoder::ArcArchiveDecoder()
    : compression_method(CompressionMethod::PlainLzss)
{
    add_arg_parser_decorator(
        ArgParserDecorator(
            [this](ArgParser &arg_parser)
            {
                auto sw = arg_parser.register_switch({"arc-key"})
                    ->set_value_name("KEY")
                    ->set_description("key used to decrypt the files");
            },
            [this](const ArgParser &arg_parser)
            {
                if (arg_parser.has_switch("arc-key"))
                    key = arg_parser.get_switch("arc-key");
            }));

    add_arg_parser_decorator(
        ArgParserDecorator(
            [this](ArgParser &arg_parser)
            {
                auto sw = arg_parser.register_switch({"arc-pack"})
                    ->set_value_name("METHOD")
                    ->set_description("compression method")
                    ->add_possible_value("plain-lzss")
                    ->add_possible_value("variable-lzss");
            },
            [this](const ArgParser &arg_parser)
            {
                if (arg_parser.has_switch("arc-pack"))
                {
                    const auto method = arg_parser.get_switch("arc-pack");
                    if (method == "plain-lzss")
                        compression_method = CompressionMethod::PlainLzss;
                    else if (method == "variable-lzss")
                        compression_method = CompressionMethod::VariableLzss;
                    else
                        throw std::logic_error("Invalid compression method");
                }
            }));
}

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    std::vector<std::function<std::unique_ptr<CustomArchiveMeta>(io::File &)>>
        meta_readers
        {
            read_meta_v0,
            read_meta_v1
        };

    for (const auto meta_reader : meta_readers)
    {
        input_file.stream.seek(magic.size());
        try
        {
            auto meta = meta_reader(input_file);
            meta->compression_method = compression_method;
            if (!key.empty())
                meta->key = key;
            return std::move(meta);
        }
        catch (const std::exception)
        {
            continue;
        }
    }

    throw err::NotSupportedError("Archive is encrypted in unknown way.");
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    if (meta->key.size())
        data = algo::unxor(data, meta->key);
    if (entry->size_orig)
    {
        if (meta->compression_method == CompressionMethod::PlainLzss)
            data = algo::pack::lzss_decompress(data, entry->size_orig);
        else if (meta->compression_method == CompressionMethod::VariableLzss)
            data = custom_lzss_decompress(data, entry->size_orig);
        else
            throw std::logic_error("Invalid compression method");
    }
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"microsoft/dds"};
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("tactics/arc");
