// SAR archive
//
// Company:   -
// Engine:    Nscripter
// Extension: .sar
//
// Known games:
// - Tsukihime

#include <cassert>
#include "formats/arc/sar_archive.h"
#include "logger.h"

namespace
{
    typedef struct
    {
        char *name;
        uint32_t offset;
        uint32_t size;
    } TableEntry;

    typedef struct SarUnpackContext
    {
        IO &arc_io;
        TableEntry *table_entry;

        SarUnpackContext(IO &arc_io) : arc_io(arc_io)
        {
        }
    } SarUnpackContext;

    std::unique_ptr<VirtualFile> sar_read_file(void *_context)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        SarUnpackContext *context = (SarUnpackContext*)_context;
        assert(context != nullptr);

        context->arc_io.seek(context->table_entry->offset);

        file->io.write_from_io(
            context->arc_io,
            context->table_entry->size);

        file->name = std::string(context->table_entry->name);
        return file;
    }
}

bool SarArchive::unpack_internal(IO &arc_io, OutputFiles &output_files)
{
    TableEntry **table;
    size_t i, j;

    uint16_t file_count = arc_io.read_u16_be();
    uint32_t offset_to_files = arc_io.read_u32_be();
    if (offset_to_files > arc_io.size())
    {
        log_error("SAR: Bad offset to files");
        return false;
    }

    table = new TableEntry*[file_count];
    assert(table != nullptr);
    for (i = 0; i < file_count; i ++)
    {
        TableEntry *entry = new TableEntry;
        assert(entry != nullptr);
        entry->name = nullptr;
        arc_io.read_until_zero(&entry->name, nullptr);
        entry->offset = arc_io.read_u32_be() + offset_to_files;
        entry->size = arc_io.read_u32_be();
        if (entry->offset + entry->size > arc_io.size())
        {
            log_error("SAR: Bad offset to file");
            for (j = 0; j < i; j ++)
                delete table[j];
            delete table;
            return false;
        }
        table[i] = entry;
    }

    SarUnpackContext context(arc_io);
    for (i = 0; i < file_count; i ++)
    {
        context.table_entry = table[i];
        output_files.save(&sar_read_file, &context);
    }

    for (i = 0; i < file_count; i ++)
    {
        delete []table[i]->name;
        delete table[i];
    }
    delete table;

    return true;
}
