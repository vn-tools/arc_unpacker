// PAK2 archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .pak
//
// Known games:
// - Saya no Uta

#include "formats/nitroplus/pak_archive.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/zlib.h"
using namespace Formats::Nitroplus;

namespace
{
    const std::string magic("\x02\x00\x00\x00", 4);

    std::unique_ptr<File> read_file(
        IO &arc_io, IO &table_io, size_t offset_to_files)
    {
        std::unique_ptr<File> file(new File);

        size_t file_name_length = table_io.read_u32_le();
        std::string file_name = table_io.read(file_name_length);
        file->name = convert_encoding(file_name, "cp932", "utf-8");

        size_t offset = table_io.read_u32_le();
        size_t size_original = table_io.read_u32_le();
        table_io.skip(4);
        size_t flags = table_io.read_u32_le();
        size_t size_compressed = table_io.read_u32_le();
        offset += offset_to_files;

        arc_io.seek(offset);
        if (flags > 0)
        {
            std::string data_uncompressed
                = zlib_inflate(arc_io.read(size_compressed));

            if (data_uncompressed.size() != size_original)
                throw std::runtime_error("Bad file size");

            file->io.write(data_uncompressed);
        }
        else
        {
            file->io.write_from_io(arc_io, size_original);
        }

        return file;
    }
}

void PakArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    if (arc_file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a PAK archive");

    uint32_t file_count = arc_file.io.read_u32_le();
    uint32_t table_size_original = arc_file.io.read_u32_le();
    uint32_t table_size_compressed = arc_file.io.read_u32_le();
    arc_file.io.skip(0x104);

    BufferedIO table_io(zlib_inflate(arc_file.io.read(table_size_compressed)));
    size_t offset_to_files = arc_file.io.tell();

    for (size_t i = 0; i < file_count; i ++)
        file_saver.save(read_file(arc_file.io, table_io, offset_to_files));
}
