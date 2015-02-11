// SAR archive
//
// Company:   -
// Engine:    Nscripter
// Extension: .sar
//
// Known games:
// - Tsukihime

#include "formats/arc/sar_archive.h"

namespace
{
    typedef struct
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
    } TableEntry;

    typedef struct UnpackContext
    {
        IO &arc_io;
        TableEntry *table_entry;

        UnpackContext(IO &arc_io) : arc_io(arc_io)
        {
        }
    } UnpackContext;

    std::unique_ptr<VirtualFile> sar_read_file(void *_context)
    {
        UnpackContext *context = (UnpackContext*)_context;

        std::unique_ptr<VirtualFile> file(new VirtualFile);
        file->name = context->table_entry->name;

        context->arc_io.seek(context->table_entry->offset);
        file->io.write_from_io(
            context->arc_io,
            context->table_entry->size);

        return file;
    }
}

void SarArchive::unpack_internal(IO &arc_io, OutputFiles &output_files) const
{
    uint16_t file_count = arc_io.read_u16_be();
    uint32_t offset_to_files = arc_io.read_u32_be();
    if (offset_to_files > arc_io.size())
        throw std::runtime_error("Bad offset to files");

    std::vector<std::unique_ptr<TableEntry>> table;
    table.reserve(file_count);
    for (size_t i = 0; i < file_count; i ++)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_io.read_until_zero();
        entry->offset = arc_io.read_u32_be() + offset_to_files;
        entry->size = arc_io.read_u32_be();
        if (entry->offset + entry->size > arc_io.size())
            throw std::runtime_error("Bad offset to file");
        table.push_back(std::move(entry));
    }

    UnpackContext context(arc_io);
    for (size_t i = 0; i < file_count; i ++)
    {
        context.table_entry = table[i].get();
        output_files.save(&sar_read_file, &context);
    }
}
