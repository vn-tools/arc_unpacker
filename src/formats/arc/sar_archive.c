// SAR archive
//
// Company:   -
// Engine:    Nscripter
// Extension: .sar
//
// Known games:
// - Tsukihime

#include <assert.h>
#include <stdlib.h>
#include "formats/arc/sar_archive.h"
#include "logger.h"

typedef struct
{
    char *name;
    uint32_t offset;
    uint32_t size;
} TableEntry;

typedef struct
{
    IO *arc_io;
    TableEntry *table_entry;
} SarUnpackContext;

static VirtualFile *sar_read_file(void *_context)
{
    VirtualFile *file = virtual_file_create();
    assert(file != NULL);

    SarUnpackContext *context = (SarUnpackContext*)_context;
    assert(context != NULL);

    io_seek(context->arc_io, context->table_entry->offset);

    io_write_string_from_io(
        file->io,
        context->arc_io,
        context->table_entry->size);

    virtual_file_set_name(file, context->table_entry->name);
    return file;
}

static bool sar_unpack(Archive *archive, IO *arc_io, OutputFiles *output_files)
{
    assert(archive != NULL);
    assert(arc_io != NULL);
    assert(output_files != NULL);

    TableEntry **table;
    size_t i, j;

    uint16_t file_count = io_read_u16_be(arc_io);
    uint32_t offset_to_files = io_read_u32_be(arc_io);
    if (offset_to_files > io_size(arc_io))
    {
        log_error("SAR: Bad offset to files");
        return false;
    }

    table = (TableEntry**)malloc(file_count * sizeof(TableEntry*));
    assert(table != NULL);
    for (i = 0; i < file_count; i ++)
    {
        TableEntry *entry = (TableEntry*)malloc(sizeof(TableEntry));
        assert(entry != NULL);
        entry->name = NULL;
        io_read_until_zero(arc_io, &entry->name, NULL);
        entry->offset = io_read_u32_be(arc_io) + offset_to_files;
        entry->size = io_read_u32_be(arc_io);
        if (entry->offset + entry->size > io_size(arc_io))
        {
            log_error("SAR: Bad offset to file");
            for (j = 0; j < i; j ++)
                free(table[j]);
            free(table);
            return false;
        }
        table[i] = entry;
    }

    SarUnpackContext context;
    context.arc_io = arc_io;
    for (i = 0; i < file_count; i ++)
    {
        context.table_entry = table[i];
        output_files_save(output_files, &sar_read_file, &context);
    }

    for (i = 0; i < file_count; i ++)
    {
        free(table[i]->name);
        free(table[i]);
    }
    free(table);

    return true;
}

Archive *sar_archive_create()
{
    Archive *archive = archive_create();
    archive->unpack = &sar_unpack;
    return archive;
}
