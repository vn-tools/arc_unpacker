// PAK2 archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .pak
//
// Known games:
// - Saya no Uta

#include "fmt/nitroplus/pak_archive.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nitroplus;

static const bstr magic = "\x02\x00\x00\x00"_b;

static std::unique_ptr<File> read_file(
    io::IO &arc_io, io::IO &table_io, size_t offset_to_files)
{
    std::unique_ptr<File> file(new File);

    size_t file_name_size = table_io.read_u32_le();
    file->name = util::sjis_to_utf8(table_io.read(file_name_size)).str();

    size_t offset = table_io.read_u32_le();
    size_t size_original = table_io.read_u32_le();
    table_io.skip(4);
    size_t flags = table_io.read_u32_le();
    size_t size_compressed = table_io.read_u32_le();
    offset += offset_to_files;

    arc_io.seek(offset);
    if (flags > 0)
    {
        auto data_compressed = arc_io.read(size_compressed);
        auto data_uncompressed = util::pack::zlib_inflate(data_compressed);

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

bool PakArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void PakArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    u32 file_count = arc_file.io.read_u32_le();
    u32 table_size_original = arc_file.io.read_u32_le();
    u32 table_size_compressed = arc_file.io.read_u32_le();
    arc_file.io.skip(0x104);

    io::BufferedIO table_io(
        util::pack::zlib_inflate(
            arc_file.io.read(table_size_compressed)));

    size_t offset_to_files = arc_file.io.tell();

    for (auto i : util::range(file_count))
        file_saver.save(read_file(arc_file.io, table_io, offset_to_files));
}

static auto dummy = fmt::Registry::add<PakArchive>("nitro/pak");
