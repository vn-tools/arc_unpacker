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

#include "dec/renpy/rpa_archive_decoder.h"
#include "algo/format.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::renpy;

namespace
{
    enum class PickleOpcode : u8
    {
        Mark           = '(',
        Stop           = '.',
        Pop            = '0',
        PopMark        = '1',
        Dup            = '2',
        Float          = 'F',
        Int            = 'I',
        BinInt1        = 'K',
        BinInt2        = 'M',
        BinInt4        = 'J',
        Long           = 'L',
        None           = 'N',
        PersId         = 'P',
        BinPersId      = 'Q',
        Reduce         = 'R',
        String         = 'S',
        BinString      = 'T',
        ShortBinString = 'U',
        Unicode        = 'V',
        BinUnicode     = 'X',
        Append         = 'a',
        Build          = 'b',
        Global         = 'c',
        Dict           = 'd',
        EmptyDict      = '}',
        Appends        = 'e',
        Get            = 'g',
        BinGet         = 'h',
        LongBinGet     = 'j',
        Inst           = 'i',
        List           = 'l',
        EmptyList      = ']',
        Obj            = 'o',
        Put            = 'p',
        BinPut         = 'q',
        LongBinPut     = 'r',
        SetItem        = 's',
        Tuple          = 't',
        EmptyTuple     = ')',
        SetItems       = 'u',
        BinFloat       = 'G',

        // Pickle protocol 2
        Proto          = '\x80'_u8,
        Newobj         = '\x81'_u8,
        Ext1           = '\x82'_u8,
        Ext2           = '\x83'_u8,
        Ext4           = '\x84'_u8,
        Tuple1         = '\x85'_u8,
        Tuple2         = '\x86'_u8,
        Tuple3         = '\x87'_u8,
        NewTrue        = '\x88'_u8,
        NewFalse       = '\x89'_u8,
        Long1          = '\x8A'_u8,
        Long4          = '\x8B'_u8,
    };
}

namespace
{
    struct UnpickleContext final
    {
        std::vector<bstr> strings;
        std::vector<int> numbers;
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        bstr prefix;
    };
}

static void unpickle_handle_string(bstr str, UnpickleContext *context)
{
    context->strings.push_back(str);
}

static void unpickle_handle_number(size_t number, UnpickleContext *context)
{
    context->numbers.push_back(number);
}

static void unpickle(io::BaseByteStream &table_stream, UnpickleContext *context)
{
    // Stupid unpickle "implementation" ahead: instead of twiddling with stack,
    // arrays, dictionaries and all that jazz, just remember all pushed strings
    // and integers for later interpretation. We also take advantage of RenPy
    // using Pickle's HIGHEST_PROTOCOL, which means there's no need to parse
    // 90% of the opcodes (such as "S" with escape stuff).
    const auto table_size = table_stream.size();
    while (table_stream.pos() < table_size)
    {
        PickleOpcode c = static_cast<PickleOpcode>(table_stream.read<u8>());
        switch (c)
        {
            case PickleOpcode::ShortBinString:
            {
                char size = table_stream.read<u8>();
                unpickle_handle_string(table_stream.read(size), context);
                break;
            }

            case PickleOpcode::BinUnicode:
            {
                const auto size = table_stream.read_le<u32>();
                unpickle_handle_string(table_stream.read(size), context);
                break;
            }

            case PickleOpcode::BinInt1:
            {
                unpickle_handle_number(table_stream.read<u8>(), context);
                break;
            }

            case PickleOpcode::BinInt2:
            {
                unpickle_handle_number(table_stream.read_le<u16>(), context);
                break;
            }

            case PickleOpcode::BinInt4:
            {
                unpickle_handle_number(table_stream.read_le<u32>(), context);
                break;
            }

            case PickleOpcode::Long1:
            {
                const auto size = table_stream.read<u8>();
                u32 number = 0;
                const auto pos = table_stream.pos();
                for (const auto i : algo::range(size))
                {
                    table_stream.seek(pos + size - 1 - i);
                    number *= 256;
                    number += table_stream.read<u8>();
                }
                unpickle_handle_number(number, context);
                table_stream.seek(pos + size);
                break;
            }

            case PickleOpcode::Proto:
                table_stream.skip(1);
                break;

            case PickleOpcode::BinPut:
                table_stream.skip(1);
                break;

            case PickleOpcode::LongBinPut:
                table_stream.skip(4);
                break;

            case PickleOpcode::Append:
            case PickleOpcode::SetItems:
            case PickleOpcode::Mark:
            case PickleOpcode::EmptyList:
            case PickleOpcode::EmptyDict:
            case PickleOpcode::Tuple1:
            case PickleOpcode::Tuple2:
            case PickleOpcode::Tuple3:
                break;

            case PickleOpcode::Stop:
                return;

            default:
            {
                throw err::NotSupportedError(algo::format(
                    "Unsupported pickle operator %c", static_cast<char>(c)));
            }
        }
    }
}

static int guess_version(io::BaseByteStream &input_stream)
{
    static const bstr magic_3 = "RPA-3.0 "_b;
    static const bstr magic_2 = "RPA-2.0 "_b;
    if (input_stream.read(magic_3.size()) == magic_3)
        return 3;
    input_stream.seek(0);
    if (input_stream.read(magic_2.size()) == magic_2)
        return 2;
    return -1;
}

static u32 read_hex_number(io::BaseByteStream &input_stream, size_t size)
{
    u32 result = 0;
    for (const auto i : algo::range(size))
    {
        char c = input_stream.read<u8>();
        result *= 16;
        if (c >= 'A' && c <= 'F')
            result += c + 10 - 'A';

        else if (c >= 'a' && c <= 'f')
            result += c + 10 - 'a';

        else if (c >= '0' && c <= '9')
            result += c - '0';
    }
    return result;
}

static bstr read_raw_table(io::BaseByteStream &input_stream)
{
    const auto size_comp = input_stream.size() - input_stream.pos();
    return algo::pack::zlib_inflate(input_stream.read(size_comp));
}

bool RpaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return guess_version(input_file.stream) >= 0;
}

std::unique_ptr<dec::ArchiveMeta> RpaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto version = guess_version(input_file.stream);
    const auto table_offset = read_hex_number(input_file.stream, 16);

    u32 key;
    if (version == 3)
    {
        input_file.stream.skip(1);
        key = read_hex_number(input_file.stream, 8);
    }
    else if (version == 2)
    {
        key = 0;
    }
    else
    {
        throw err::UnsupportedVersionError(version);
    }

    input_file.stream.seek(table_offset);
    io::MemoryByteStream table_stream(read_raw_table(input_file.stream));

    UnpickleContext context;
    unpickle(table_stream, &context);

    // Suspicion: reading renpy sources leaves me under impression that
    // older games might not embed prefixes at all. This means that there
    // are twice as many numbers as strings, and all prefixes should be set
    // to empty.  Since I haven't seen such games, I leave this remark only
    // as a comment.
    if (context.strings.size() % 2 != 0)
        throw err::NotSupportedError("Unsupported table format");
    if (context.numbers.size() != context.strings.size())
        throw err::NotSupportedError("Unsupported table format");

    const auto file_count = context.strings.size() / 2;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = context.strings[i * 2 ].str();
        entry->prefix = context.strings[i * 2 + 1];
        entry->offset = context.numbers[i * 2] ^ key;
        entry->size = context.numbers[i * 2 + 1] ^ key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> RpaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, entry->prefix + data);
}

static auto _ = dec::register_decoder<RpaArchiveDecoder>("renpy/rpa");
