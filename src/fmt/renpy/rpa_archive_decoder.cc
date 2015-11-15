#include "fmt/renpy/rpa_archive_decoder.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::renpy;

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

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
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

static void unpickle(io::IO &table_io, UnpickleContext *context)
{
    // Stupid unpickle "implementation" ahead: instead of twiddling with stack,
    // arrays, dictionaries and all that jazz, just remember all pushed strings
    // and integers for later interpretation. We also take advantage of RenPy
    // using Pickle's HIGHEST_PROTOCOL, which means there's no need to parse
    // 90% of the opcodes (such as "S" with escape stuff).
    size_t table_size = table_io.size();
    while (table_io.tell() < table_size)
    {
        PickleOpcode c = static_cast<PickleOpcode>(table_io.read_u8());
        switch (c)
        {
            case PickleOpcode::ShortBinString:
            {
                char size = table_io.read_u8();
                unpickle_handle_string(table_io.read(size), context);
                break;
            }

            case PickleOpcode::BinUnicode:
            {
                u32 size = table_io.read_u32_le();
                unpickle_handle_string(table_io.read(size), context);
                break;
            }

            case PickleOpcode::BinInt1:
            {
                unpickle_handle_number(table_io.read_u8(), context);
                break;
            }

            case PickleOpcode::BinInt2:
            {
                unpickle_handle_number(table_io.read_u16_le(), context);
                break;
            }

            case PickleOpcode::BinInt4:
            {
                unpickle_handle_number(table_io.read_u32_le(), context);
                break;
            }

            case PickleOpcode::Long1:
            {
                size_t size = table_io.read_u8();
                u32 number = 0;
                size_t pos = table_io.tell();
                for (auto i : util::range(size))
                {
                    table_io.seek(pos + size - 1 - i);
                    number *= 256;
                    number += table_io.read_u8();
                }
                unpickle_handle_number(number, context);
                table_io.seek(pos + size);
                break;
            }

            case PickleOpcode::Proto:
                table_io.skip(1);
                break;

            case PickleOpcode::BinPut:
                table_io.skip(1);
                break;

            case PickleOpcode::LongBinPut:
                table_io.skip(4);
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
                throw err::NotSupportedError(util::format(
                    "Unsupported pickle operator %c", static_cast<char>(c)));
            }
        }
    }
}

static int guess_version(io::IO &arc_io)
{
    static const bstr magic_3 = "RPA-3.0 "_b;
    static const bstr magic_2 = "RPA-2.0 "_b;
    if (arc_io.read(magic_3.size()) == magic_3)
        return 3;
    arc_io.seek(0);
    if (arc_io.read(magic_2.size()) == magic_2)
        return 2;
    return -1;
}

static u32 read_hex_number(io::IO &arc_io, size_t size)
{
    u32 result = 0;
    for (auto i : util::range(size))
    {
        char c = arc_io.read_u8();
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

static bstr read_raw_table(io::IO &arc_io)
{
    size_t compressed_size = arc_io.size() - arc_io.tell();
    return util::pack::zlib_inflate(arc_io.read(compressed_size));
}

bool RpaArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return guess_version(arc_file.io) >= 0;
}

std::unique_ptr<fmt::ArchiveMeta>
    RpaArchiveDecoder::read_meta_impl(File &arc_file) const
{
    int version = guess_version(arc_file.io);
    size_t table_offset = read_hex_number(arc_file.io, 16);

    u32 key;
    if (version == 3)
    {
        arc_file.io.skip(1);
        key = read_hex_number(arc_file.io, 8);
    }
    else if (version == 2)
    {
        key = 0;
    }
    else
    {
        throw err::UnsupportedVersionError(version);
    }

    arc_file.io.seek(table_offset);
    io::BufferedIO table_io(read_raw_table(arc_file.io));

    UnpickleContext context;
    unpickle(table_io, &context);

    // Suspicion: reading renpy sources leaves me under impression that
    // older games might not embed prefixes at all. This means that there
    // are twice as many numbers as strings, and all prefixes should be set
    // to empty.  Since I haven't seen such games, I leave this remark only
    // as a comment.
    if (context.strings.size() % 2 != 0)
        throw err::NotSupportedError("Unsupported table format");
    if (context.numbers.size() != context.strings.size())
        throw err::NotSupportedError("Unsupported table format");

    size_t file_count = context.strings.size() / 2;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = context.strings[i * 2 ].str();
        entry->prefix = context.strings[i * 2 + 1];
        entry->offset = context.numbers[i * 2] ^ key;
        entry->size = context.numbers[i * 2 + 1] ^ key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> RpaArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = arc_file.io.seek(entry->offset).read(entry->size);
    return std::make_unique<File>(entry->name, entry->prefix + data);
}

static auto dummy = fmt::register_fmt<RpaArchiveDecoder>("renpy/rpa");
