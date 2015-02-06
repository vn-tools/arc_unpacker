// PAK2 archive
//
// Engine:    -
// Company:   Nitroplus
// Extension: .pak
//
// Known games:
// - Saya no Uta

#include <cassert>
#include <cstring>
#include "formats/arc/pak_archive.h"
#include "logger.h"
#include "string_ex.h"

const char *pak_magic = "\x02\x00\x00\x00";
const size_t pak_magic_length = 4;

namespace
{
    typedef struct
    {
        IO *arc_io;
        IO *table_io;
        size_t offset_to_files;
    } PakUnpackContext;

    bool pak_check_magic(IO *arc_io)
    {
        char magic[pak_magic_length];
        io_read_string(arc_io, magic, pak_magic_length);
        return memcmp(magic, pak_magic, pak_magic_length) == 0;
    }

    char *pak_read_zlib(
        IO *arc_io, size_t size_compressed, size_t *size_uncompressed)
    {
        char *data_compressed = new char[size_compressed];
        assert(data_compressed != nullptr);

        if (!io_read_string(arc_io, data_compressed, size_compressed))
            assert(0);

        char *data_uncompressed;
        if (!zlib_inflate(
            data_compressed,
            size_compressed,
            &data_uncompressed,
            size_uncompressed))
        {
            assert(0);
        }
        assert(data_uncompressed != nullptr);
        delete []data_compressed;

        return data_uncompressed;
    }

    std::unique_ptr<VirtualFile> pak_read_file(void *context)
    {
        PakUnpackContext *unpack_context = (PakUnpackContext*)context;
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        size_t file_name_length = io_read_u32_le(unpack_context->table_io);
        char *file_name = new char[file_name_length];
        assert(file_name != nullptr);
        io_read_string(unpack_context->table_io, file_name, file_name_length);

        char *file_name_utf8 = nullptr;
        if (!convert_encoding(
            file_name, file_name_length,
            &file_name_utf8, nullptr,
            "cp932", "utf-8"))
        {
            assert(0);
        }
        assert(file_name_utf8 != nullptr);
        file->name = std::string(file_name_utf8);
        delete []file_name_utf8;

        size_t offset = io_read_u32_le(unpack_context->table_io);
        size_t size_original = io_read_u32_le(unpack_context->table_io);
        io_skip(unpack_context->table_io, 4);
        size_t flags = io_read_u32_le(unpack_context->table_io);
        size_t size_compressed = io_read_u32_le(unpack_context->table_io);
        offset += unpack_context->offset_to_files;

        io_seek(unpack_context->arc_io, offset);
        if (flags > 0)
        {
            size_t size_uncompressed = 0;
            char *data_uncompressed = pak_read_zlib(
                unpack_context->arc_io, size_compressed, &size_uncompressed);
            assert(data_uncompressed != nullptr);
            io_write_string(&file->io, data_uncompressed, size_original);
            delete []data_uncompressed;
            if (size_uncompressed != size_original)
            {
                log_error("PAK: Bad file size");
                return nullptr;
            }
        }
        else
        {
            io_write_string_from_io(
                &file->io, unpack_context->arc_io, size_original);
        }

        delete []file_name;
        return file;
    }
}

bool PakArchive::unpack_internal(IO *arc_io, OutputFiles &output_files)
{
    if (!pak_check_magic(arc_io))
    {
        log_error("PAK: Not a PAK archive");
        return false;
    }

    uint32_t file_count = io_read_u32_le(arc_io);
    uint32_t table_size_original = io_read_u32_le(arc_io);
    uint32_t table_size_compressed = io_read_u32_le(arc_io);
    io_skip(arc_io, 0x104);

    char *table = pak_read_zlib(arc_io, table_size_compressed, nullptr);
    assert(table != nullptr);

    IO *table_io = io_create_from_buffer(table, table_size_original);
    assert(table_io != nullptr);
    delete []table;

    size_t i;
    PakUnpackContext unpack_context;
    unpack_context.arc_io = arc_io;
    unpack_context.table_io = table_io;
    unpack_context.offset_to_files = io_tell(arc_io);
    for (i = 0; i < file_count; i ++)
    {
        output_files.save(&pak_read_file, &unpack_context);
    }

    io_destroy(table_io);
    return true;
}
