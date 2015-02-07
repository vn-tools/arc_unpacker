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
#include "buffered_io.h"
#include "logger.h"
#include "string_ex.h"

const char *pak_magic = "\x02\x00\x00\x00";
const size_t pak_magic_length = 4;

namespace
{
    typedef struct PakUnpackContext
    {
        IO &arc_io;
        IO &table_io;
        size_t offset_to_files;

        PakUnpackContext(IO &arc_io, IO &table_io)
            : arc_io(arc_io), table_io(table_io)
        {
        }
    } PakUnpackContext;

    bool pak_check_magic(IO &arc_io)
    {
        char magic[pak_magic_length];
        arc_io.read(magic, pak_magic_length);
        return memcmp(magic, pak_magic, pak_magic_length) == 0;
    }

    std::unique_ptr<VirtualFile> pak_read_file(void *context)
    {
        PakUnpackContext *unpack_context = (PakUnpackContext*)context;
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        size_t file_name_length = unpack_context->table_io.read_u32_le();
        char *file_name = new char[file_name_length];
        assert(file_name != nullptr);
        unpack_context->table_io.read(file_name, file_name_length);

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

        size_t offset = unpack_context->table_io.read_u32_le();
        size_t size_original = unpack_context->table_io.read_u32_le();
        unpack_context->table_io.skip(4);
        size_t flags = unpack_context->table_io.read_u32_le();
        size_t size_compressed = unpack_context->table_io.read_u32_le();
        offset += unpack_context->offset_to_files;

        unpack_context->arc_io.seek(offset);
        if (flags > 0)
        {
            std::string data_uncompressed
                = zlib_inflate(unpack_context->arc_io.read(size_compressed));
            if (data_uncompressed.size() != size_original)
            {
                log_error("PAK: Bad file size");
                return nullptr;
            }
            file->io.write(data_uncompressed);
        }
        else
        {
            file->io.write_from_io(unpack_context->arc_io, size_original);
        }

        delete []file_name;
        return file;
    }
}

bool PakArchive::unpack_internal(IO &arc_io, OutputFiles &output_files)
{
    if (!pak_check_magic(arc_io))
    {
        log_error("PAK: Not a PAK archive");
        return false;
    }

    uint32_t file_count = arc_io.read_u32_le();
    __attribute__((unused)) uint32_t table_size_original = arc_io.read_u32_le();
    uint32_t table_size_compressed = arc_io.read_u32_le();
    arc_io.skip(0x104);

    BufferedIO table_io(zlib_inflate(arc_io.read(table_size_compressed)));

    size_t i;
    PakUnpackContext unpack_context(arc_io, table_io);
    unpack_context.offset_to_files = arc_io.tell();
    for (i = 0; i < file_count; i ++)
    {
        output_files.save(&pak_read_file, &unpack_context);
    }

    return true;
}
