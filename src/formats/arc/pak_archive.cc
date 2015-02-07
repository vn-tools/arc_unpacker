// PAK2 archive
//
// Engine:    -
// Company:   Nitroplus
// Extension: .pak
//
// Known games:
// - Saya no Uta

#include "buffered_io.h"
#include "formats/arc/pak_archive.h"
#include "string_ex.h"

const std::string pak_magic("\x02\x00\x00\x00", 4);

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

    std::unique_ptr<VirtualFile> pak_read_file(void *context)
    {
        PakUnpackContext *unpack_context = (PakUnpackContext*)context;
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        size_t file_name_length = unpack_context->table_io.read_u32_le();
        std::string file_name = unpack_context->table_io.read(file_name_length);
        file->name = convert_encoding(file_name, "cp932", "utf-8");

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
                throw std::runtime_error("Bad file size");

            file->io.write(data_uncompressed);
        }
        else
        {
            file->io.write_from_io(unpack_context->arc_io, size_original);
        }

        return file;
    }
}

void PakArchive::unpack_internal(IO &arc_io, OutputFiles &output_files) const
{
    if (arc_io.read(pak_magic.size()) != pak_magic)
        throw std::runtime_error("Not a PAK archive");

    uint32_t file_count = arc_io.read_u32_le();
    __attribute__((unused)) uint32_t table_size_original = arc_io.read_u32_le();
    uint32_t table_size_compressed = arc_io.read_u32_le();
    arc_io.skip(0x104);

    BufferedIO table_io(zlib_inflate(arc_io.read(table_size_compressed)));

    size_t i;
    PakUnpackContext unpack_context(arc_io, table_io);
    unpack_context.offset_to_files = arc_io.tell();
    for (i = 0; i < file_count; i ++)
        output_files.save(&pak_read_file, &unpack_context);
}
