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

#include "buffered_io.h"
#include "formats/arc/rpa_archive.h"
#include "string/zlib.h"

// Stupid unpickle "implementation" ahead: instead of twiddling with stack,
// arrays, dictionaries and all that crap, just remember all pushed strings
// and integers for later interpretation.  We also take advantage of RenPy
// using Pickle's HIGHEST_PROTOCOL, which means there's no need to parse
// 90% of the opcodes (such as "S" with escape stuff).
namespace
{
    #define PICKLE_MARK            '('
    #define PICKLE_STOP            '.'
    #define PICKLE_POP             '0'
    #define PICKLE_POP_MARK        '1'
    #define PICKLE_DUP             '2'
    #define PICKLE_FLOAT           'F'
    #define PICKLE_INT             'I'
    #define PICKLE_BININT1         'K'
    #define PICKLE_BININT2         'M'
    #define PICKLE_BININT4         'J'
    #define PICKLE_LONG            'L'
    #define PICKLE_NONE            'N'
    #define PICKLE_PERSID          'P'
    #define PICKLE_BINPERSID       'Q'
    #define PICKLE_REDUCE          'R'
    #define PICKLE_STRING          'S'
    #define PICKLE_BINSTRING       'T'
    #define PICKLE_SHORT_BINSTRING 'U'
    #define PICKLE_UNICODE         'V'
    #define PICKLE_BINUNICODE      'X'
    #define PICKLE_APPEND          'a'
    #define PICKLE_BUILD           'b'
    #define PICKLE_GLOBAL          'c'
    #define PICKLE_DICT            'd'
    #define PICKLE_EMPTY_DICT      '}'
    #define PICKLE_APPENDS         'e'
    #define PICKLE_GET             'g'
    #define PICKLE_BINGET          'h'
    #define PICKLE_LONG_BINGET     'j'
    #define PICKLE_INST            'i'
    #define PICKLE_LIST            'l'
    #define PICKLE_EMPTY_LIST      ']'
    #define PICKLE_OBJ             'o'
    #define PICKLE_PUT             'p'
    #define PICKLE_BINPUT          'q'
    #define PICKLE_LONG_BINPUT     'r'
    #define PICKLE_SETITEM         's'
    #define PICKLE_TUPLE           't'
    #define PICKLE_EMPTY_TUPLE     ')'
    #define PICKLE_SETITEMS        'u'
    #define PICKLE_BINFLOAT        'G'

    // Pickle protocol 2
    #define PICKLE_PROTO    (unsigned char)'\x80'
    #define PICKLE_NEWOBJ   (unsigned char)'\x81'
    #define PICKLE_EXT1     (unsigned char)'\x82'
    #define PICKLE_EXT2     (unsigned char)'\x83'
    #define PICKLE_EXT4     (unsigned char)'\x84'
    #define PICKLE_TUPLE1   (unsigned char)'\x85'
    #define PICKLE_TUPLE2   (unsigned char)'\x86'
    #define PICKLE_TUPLE3   (unsigned char)'\x87'
    #define PICKLE_NEWTRUE  (unsigned char)'\x88'
    #define PICKLE_NEWFALSE (unsigned char)'\x89'
    #define PICKLE_LONG1    (unsigned char)'\x8a'
    #define PICKLE_LONG4    (unsigned char)'\x8b'

    typedef struct
    {
        std::vector<std::string> strings;
        std::vector<int> numbers;
    } UnpickleContext;

    void unpickle_handle_string(
        std::string str, UnpickleContext *context)
    {
        context->strings.push_back(str);
    }

    void unpickle_handle_number(size_t number, UnpickleContext *context)
    {
        context->numbers.push_back(number);
    }

    void unpickle(IO &table_io, UnpickleContext *context)
    {
        size_t table_size = table_io.size();
        while (table_io.tell() < table_size)
        {
            unsigned char c = table_io.read_u8();
            switch (c)
            {
                case PICKLE_SHORT_BINSTRING:
                {
                    char size = table_io.read_u8();
                    unpickle_handle_string(table_io.read(size), context);
                    break;
                }

                case PICKLE_BINUNICODE:
                {
                    uint32_t size = table_io.read_u32_le();
                    unpickle_handle_string(table_io.read(size), context);
                    break;
                }

                case PICKLE_BININT1:
                {
                    unpickle_handle_number(table_io.read_u8(), context);
                    break;
                }

                case PICKLE_BININT2:
                {
                    unpickle_handle_number(table_io.read_u16_le(), context);
                    break;
                }

                case PICKLE_BININT4:
                {
                    unpickle_handle_number(table_io.read_u32_le(), context);
                    break;
                }

                case PICKLE_LONG1:
                {
                    size_t length = table_io.read_u8();
                    uint32_t number = 0;
                    size_t i;
                    size_t pos = table_io.tell();
                    for (i = 0; i < length; i ++)
                    {
                        table_io.seek(pos + length - 1 - i);
                        number *= 256;
                        number += table_io.read_u8();
                    }
                    unpickle_handle_number(number, context);
                    table_io.seek(pos + length);
                    break;
                }

                case PICKLE_PROTO:
                    table_io.skip(1);
                    break;

                case PICKLE_BINPUT:
                    table_io.skip(1);
                    break;

                case PICKLE_LONG_BINPUT:
                    table_io.skip(4);
                    break;

                case PICKLE_APPEND:
                case PICKLE_SETITEMS:
                case PICKLE_MARK:
                case PICKLE_EMPTY_LIST:
                case PICKLE_EMPTY_DICT:
                case PICKLE_TUPLE1:
                case PICKLE_TUPLE2:
                case PICKLE_TUPLE3:
                    break;

                case PICKLE_STOP:
                    return;

                default:
                {
                    std::string msg = "Unsupported pickle operator ";
                    msg += c;
                    throw std::runtime_error(msg);
                }
            }
        }
    }
}

namespace
{
    typedef struct
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
        std::string prefix;
        size_t prefix_size;
    } TableEntry;

    std::vector<std::unique_ptr<TableEntry>> decode_table(
        IO &table_io, uint32_t key)
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
        std::vector<std::unique_ptr<TableEntry>> entries;
        entries.reserve(file_count);

        for (size_t i = 0; i < file_count; i ++)
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

    int check_version(IO &arc_io)
    {
        const std::string magic_3("RPA-3.0 ", 8);
        const std::string magic_2("RPA-2.0 ", 8);
        if (arc_io.read(magic_3.size()) == magic_3)
            return 3;
        arc_io.seek(0);
        if (arc_io.read(magic_2.size()) == magic_2)
            return 2;
        return -1;
    }

    uint32_t read_hex_number(IO &arc_io, size_t length)
    {
        size_t i;
        uint32_t result = 0;
        for (i = 0; i < length; i ++)
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

    std::string read_raw_table(IO &arc_io)
    {
        size_t compressed_size = arc_io.size() - arc_io.tell();
        std::string compressed = arc_io.read(compressed_size);
        std::string uncompressed = zlib_inflate(compressed);
        return uncompressed;
    }

    std::unique_ptr<File> read_file(
        IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);

        arc_io.seek(table_entry.offset);

        file->io.write(table_entry.prefix);
        file->io.write_from_io(arc_io, table_entry.size);

        file->name = table_entry.name;
        return file;
    }
}

void RpaArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    int version = check_version(arc_file.io);
    size_t table_offset = read_hex_number(arc_file.io, 16);

    uint32_t key;
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
        throw std::runtime_error("Not a RPA archive");
    }

    arc_file.io.seek(table_offset);
    BufferedIO table_io(read_raw_table(arc_file.io));
    auto table = decode_table(table_io, key);

    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
