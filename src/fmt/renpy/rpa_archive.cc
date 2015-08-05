// RPA archive
//
// Company:   -
// Engine:    Ren'Py
// Extension: .rpa
//
// Known games:
// - Everlasting Summer
// - Katawa Shoujo
// - Long Live The Queen

#include "fmt/renpy/rpa_archive.h"
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
    struct UnpickleContext
    {
        std::vector<std::string> strings;
        std::vector<int> numbers;
    };

    struct TableEntry
    {
        std::string name;
        u32 offset;
        u32 size;
        std::string prefix;
        size_t prefix_size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static void unpickle_handle_string(std::string str, UnpickleContext *context)
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
    // arrays, dictionaries and all that crap, just remember all pushed strings
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
                size_t length = table_io.read_u8();
                u32 number = 0;
                size_t pos = table_io.tell();
                for (auto i : util::range(length))
                {
                    table_io.seek(pos + length - 1 - i);
                    number *= 256;
                    number += table_io.read_u8();
                }
                unpickle_handle_number(number, context);
                table_io.seek(pos + length);
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
                throw std::runtime_error(util::format(
                    "Unsupported pickle operator %c", static_cast<char>(c)));
            }
        }
    }
}

static Table decode_table(io::IO &table_io, u32 key)
{
    UnpickleContext context;
    unpickle(table_io, &context);

    // Suspicion: reading renpy sources leaves me under impression that
    // older games might not embed prefixes at all. This means that there
    // are twice as many numbers as strings, and all prefixes should be set
    // to empty.  Since I haven't seen such games, I leave this remark only
    // as a comment.
    if (context.strings.size() % 2 != 0)
        throw std::runtime_error("Unsupported table format");
    if (context.numbers.size() != context.strings.size())
        throw std::runtime_error("Unsupported table format");

    size_t file_count = context.strings.size() / 2;
    Table entries;
    entries.reserve(file_count);

    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = context.strings[i * 2 ];
        entry->prefix = context.strings[i * 2 + 1];
        entry->offset = context.numbers[i * 2] ^ key;
        entry->size = context.numbers[i * 2 + 1] ^ key;
        entries.push_back(std::move(entry));
    }
    return entries;
}

static int guess_version(io::IO &arc_io)
{
    const std::string magic_3 = "RPA-3.0 "_s;
    const std::string magic_2 = "RPA-2.0 "_s;
    if (arc_io.read(magic_3.size()) == magic_3)
        return 3;
    arc_io.seek(0);
    if (arc_io.read(magic_2.size()) == magic_2)
        return 2;
    return -1;
}

static u32 read_hex_number(io::IO &arc_io, size_t length)
{
    u32 result = 0;
    for (auto i : util::range(length))
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

static std::string read_raw_table(io::IO &arc_io)
{
    size_t compressed_size = arc_io.size() - arc_io.tell();
    std::string compressed = arc_io.read(compressed_size);
    std::string uncompressed = util::pack::zlib_inflate(compressed);
    return uncompressed;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);

    arc_io.seek(entry.offset);

    file->io.write(entry.prefix);
    file->io.write_from_io(arc_io, entry.size);

    file->name = entry.name;
    return file;
}

bool RpaArchive::is_recognized_internal(File &arc_file) const
{
    return guess_version(arc_file.io) >= 0;
}

void RpaArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
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
        throw std::runtime_error("Unknown RPA version");
    }

    arc_file.io.seek(table_offset);
    io::BufferedIO table_io(read_raw_table(arc_file.io));
    auto table = decode_table(table_io, key);

    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}
