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

#include <cassert>
#include <cstring>
#include "formats/arc/rpa_archive.h"
#include "io.h"
#include "logger.h"
#include "string_ex.h"

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
        std::vector<size_t> string_lengths;
        std::vector<std::string> strings;
        std::vector<int> numbers;
    } RpaUnpickleContext;

    void rpa_unpickle_handle_string(
        char *str, size_t str_size, RpaUnpickleContext *context)
    {
        context->strings.push_back(std::string(str));
        context->string_lengths.push_back(str_size);
        delete []str;
    }

    void rpa_unpickle_handle_number(size_t number, RpaUnpickleContext *context)
    {
        context->numbers.push_back(number);
    }

    char *rpa_unpickle_read_string(IO *table_io, size_t str_size)
    {
        char *str = new char[str_size + 1];
        assert(str != nullptr);
        io_read_string(table_io, str, str_size);
        str[str_size] = '\0';
        return str;
    }

    bool rpa_unpickle(IO *table_io, RpaUnpickleContext *context)
    {
        size_t table_size = io_size(table_io);
        while (io_tell(table_io) < table_size)
        {
            unsigned char c = io_read_u8(table_io);
            switch (c)
            {
                case PICKLE_SHORT_BINSTRING:
                {
                    char str_size = io_read_u8(table_io);
                    char *string = rpa_unpickle_read_string(table_io, str_size);
                    rpa_unpickle_handle_string(string, str_size, context);
                    break;
                }

                case PICKLE_BINUNICODE:
                {
                    uint32_t str_size = io_read_u32_le(table_io);
                    char *string = rpa_unpickle_read_string(table_io, str_size);
                    rpa_unpickle_handle_string(string, str_size, context);
                    break;
                }

                case PICKLE_BININT1:
                {
                    rpa_unpickle_handle_number(
                        io_read_u8(table_io), context);
                    break;
                }

                case PICKLE_BININT2:
                {
                    rpa_unpickle_handle_number(
                        io_read_u16_le(table_io), context);
                    break;
                }

                case PICKLE_BININT4:
                {
                    rpa_unpickle_handle_number(
                        io_read_u32_le(table_io), context);
                    break;
                }

                case PICKLE_LONG1:
                {
                    size_t length = io_read_u8(table_io);
                    uint32_t number = 0;
                    size_t i;
                    size_t pos = io_tell(table_io);
                    for (i = 0; i < length; i ++)
                    {
                        io_seek(table_io, pos + length - 1 - i);
                        number *= 256;
                        number += io_read_u8(table_io);
                    }
                    rpa_unpickle_handle_number(number, context);
                    io_seek(table_io, pos + length);
                    break;
                }

                case PICKLE_PROTO:
                    io_skip(table_io, 1);
                    break;

                case PICKLE_BINPUT:
                    io_skip(table_io, 1);
                    break;

                case PICKLE_LONG_BINPUT:
                    io_skip(table_io, 4);
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
                    return true;

                default:
                    log_error(
                        "RPA: Unsupported pickle operator '%c' (%02X)", c, c);
                    return false;
            }
        }
        return false;
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
    } RpaTableEntry;

    typedef struct
    {
        IO *arc_io;
        RpaTableEntry *table_entry;
    } RpaUnpackContext;

    RpaTableEntry **rpa_decode_table(
        const char *table,
        size_t table_size,
        uint32_t key,
        size_t *file_count)
    {
        RpaUnpickleContext context;

        IO *table_io = io_create_from_buffer(table, table_size);
        rpa_unpickle(table_io, &context);

        // Suspicion: reading renpy sources leaves me under impression that
        // older games might not embed prefixes at all. This means that there
        // are twice as many numbers as strings, and all prefixes should be set
        // to empty.  Since I haven't seen such games, I leave this remark only
        // as a comment.
        assert(context.strings.size() % 2 == 0);
        *file_count = context.strings.size() / 2;
        assert(context.numbers.size() == context.strings.size());

        RpaTableEntry **entries = new RpaTableEntry*[*file_count];
        assert(entries != nullptr);

        size_t i;
        for (i = 0; i < *file_count; i ++)
        {
            RpaTableEntry *entry = new RpaTableEntry;
            assert(entry != nullptr);
            entry->name = context.strings[i * 2 ];
            entry->prefix = context.strings[i * 2 + 1];
            entry->prefix_size = context.string_lengths[i * 2 + 1];
            entry->offset = context.numbers[i * 2] ^ key;
            entry->size = context.numbers[i * 2 + 1] ^ key;
            entries[i] = entry;
        }

        io_destroy(table_io);
        return entries;
    }

    int rpa_check_version(IO *arc_io)
    {
        const char *rpa_magic_3 = "RPA-3.0 ";
        const char *rpa_magic_2 = "RPA-2.0 ";
        char magic[8];
        io_read_string(arc_io, magic, 8);
        if (memcmp(rpa_magic_2, magic, 8) == 0)
            return 2;
        if (memcmp(rpa_magic_3, magic, 8) == 0)
            return 3;
        return -1;
    }

    uint32_t rpa_read_hex_number(IO *arc_io, size_t length)
    {
        size_t i;
        uint32_t result = 0;
        for (i = 0; i < length; i ++)
        {
            char c = io_read_u8(arc_io);
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

    bool rpa_read_raw_table(IO *arc_io, char **table, size_t *table_size)
    {
        size_t compressed_size = io_size(arc_io) - io_tell(arc_io);
        char *compressed = new char[compressed_size];
        assert(compressed != nullptr);
        if (!io_read_string(arc_io, compressed, compressed_size))
            assert(0);

        *table = nullptr;
        bool result = zlib_inflate(
            compressed, compressed_size, table, table_size);
        if (!result)
        {
            delete *table;
            *table = nullptr;
            *table_size = 0;
        }
        delete []compressed;
        return result;
    }

    VirtualFile *rpa_read_file(void *_context)
    {
        RpaUnpackContext *context = (RpaUnpackContext*)_context;
        assert(context != nullptr);
        VirtualFile *file = virtual_file_create();
        assert(file != nullptr);

        if (!io_seek(context->arc_io, context->table_entry->offset))
            assert(0);
        assert(context->table_entry->offset < io_size(context->arc_io));

        if (!io_write_string(
            file->io,
            context->table_entry->prefix.c_str(),
            context->table_entry->prefix_size))
        {
            assert(0);
        }

        if (!io_write_string_from_io(
            file->io,
            context->arc_io,
            context->table_entry->size))
        {
            assert(0);
        }

        virtual_file_set_name(file, context->table_entry->name.c_str());
        return file;
    }
}

bool RpaArchive::unpack_internal(IO *arc_io, OutputFiles &output_files)
{
    int version = rpa_check_version(arc_io);

    size_t table_offset;
    uint32_t key;
    if (version == 3)
    {
        table_offset = rpa_read_hex_number(arc_io, 16);
        io_skip(arc_io, 1);
        key = rpa_read_hex_number(arc_io, 8);
    }
    else if (version == 2)
    {
        table_offset = rpa_read_hex_number(arc_io, 16);
        key = 0;
    }
    else
    {
        log_error("RPA: Not a RPA archive");
        return false;
    }

    log_info("RPA: Version: %d", version);

    if (!io_seek(arc_io, table_offset))
    {
        log_error("RPA: Bad table offset");
        return false;
    }

    size_t table_size;
    char *table;
    if (!rpa_read_raw_table(arc_io, &table, &table_size))
    {
        log_error("RPA: Failed to read table");
        return false;
    }

    size_t file_count;
    RpaTableEntry **entries = rpa_decode_table(
        table, table_size, key, &file_count);
    assert(entries != nullptr);
    delete []table;

    size_t i;
    RpaUnpackContext context;
    context.arc_io = arc_io;
    for (i = 0; i < file_count; i ++)
    {
        context.table_entry = entries[i];
        output_files.save(&rpa_read_file, &context);
        delete entries[i];
    }
    delete []entries;
    return true;
}
