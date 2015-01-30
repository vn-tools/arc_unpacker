#include <stdlib.h>
#include "assert_ex.h"
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
    IO *io;
    TableEntry *table_entry;
} Context;

static VirtualFile *read_file(void *_context)
{
    VirtualFile *vf = vf_create();
    Context *context = (Context*)_context;
    io_seek(context->io, context->table_entry->offset);
    char *data = io_read_string(context->io, context->table_entry->size);
    assert_not_null(data);
    vf_set_data(vf, data, context->table_entry->size);
    vf_set_name(vf, context->table_entry->name);
    free(data);
    return vf;
}

static bool unpack(Archive *archive, IO *io, OutputFiles *output_files)
{
    TableEntry **table;
    size_t i, j;

    assert_not_null(archive);
    assert_not_null(io);
    assert_not_null(output_files);

    uint16_t file_count = io_read_u16_be(io);
    uint32_t offset_to_files = io_read_u32_be(io);
    if (offset_to_files > io_size(io))
    {
        log_error("Bad offset to files");
        return false;
    }

    table = (TableEntry**)malloc(file_count * sizeof(TableEntry*));
    assert_not_null(table);
    for (i = 0; i < file_count; i ++)
    {
        TableEntry *entry = (TableEntry*)malloc(sizeof(TableEntry));
        assert_not_null(entry);
        entry->name = io_read_until_zero(io);
        entry->offset = io_read_u32_be(io) + offset_to_files;
        entry->size = io_read_u32_be(io);
        if (entry->offset + entry->size > io_size(io))
        {
            log_error("Bad offset to file");
            for (j = 0; j < i; j ++)
                free(table[j]);
            free(table);
            return false;
        }
        table[i] = entry;
    }

    Context context;
    context.io = io;
    for (i = 0; i < file_count; i ++)
    {
        context.table_entry = table[i];
        output_files_save(output_files, &read_file, &context);
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
    archive->unpack = &unpack;
    return archive;
}
