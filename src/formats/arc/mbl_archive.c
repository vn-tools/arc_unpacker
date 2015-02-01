#include <stdlib.h>
#include <string.h>
#include "formats/arc/mbl_archive.h"
#include "formats/gfx/prs_converter.h"
#include "assert_ex.h"
#include "logger.h"
#include "string_ex.h"

typedef struct
{
    Archive *archive;
    IO *io;

    char *name;
    uint32_t offset;
    uint32_t size;
} TableEntry;

static int mbl_get_version(IO *io)
{
    uint32_t file_count = io_read_u32_le(io);
    if (!io_skip(io, file_count * (16 + 8) - 8))
        return -1;
    uint32_t last_file_offset = io_read_u32_le(io);
    uint32_t last_file_size = io_read_u32_le(io);
    io_seek(io, 0);
    return last_file_offset + last_file_size == io_size(io) ? 1 : 2;
}

static VirtualFile *mbl_read_file(void *context)
{
    VirtualFile *vf = vf_create();
    TableEntry *table_entry = (TableEntry*)context;
    io_seek(table_entry->io, table_entry->offset);
    char *data = (char*)malloc(table_entry->size);
    assert_not_null(data);
    io_read_string(table_entry->io, data, table_entry->size);
    vf_set_data(vf, data, table_entry->size);
    vf_set_name(vf, table_entry->name);
    if (memcmp(data, prs_magic, prs_magic_length) == 0)
        converter_decode((Converter*)table_entry->archive->data, vf);
    free(data);
    return vf;
}

static bool mbl_unpack(
    Archive *archive,
    IO *io,
    OutputFiles *output_files)
{
    size_t i, old_pos;
    int version = mbl_get_version(io);
    if (version == -1)
    {
        log_error("Not a MBL archive");
        return false;
    }
    log_info("Version: %d", version);

    uint32_t file_count = io_read_u32_le(io);
    uint32_t name_length = version == 2 ? io_read_u32_le(io) : 16;
    for (i = 0; i < file_count; i ++)
    {
        TableEntry *entry = (TableEntry*)malloc(sizeof(TableEntry));
        assert_not_null(entry);

        old_pos = io_tell(io);
        char *tmp_name = NULL;
        io_read_until_zero(io, &tmp_name, NULL);
        assert_not_null(tmp_name);
        assert_that(convert_encoding(
            tmp_name, strlen(tmp_name),
            &entry->name, NULL,
            "sjis", "utf-8"));
        free(tmp_name);
        io_seek(io, old_pos + name_length);

        entry->archive = archive;
        entry->offset = io_read_u32_le(io);
        entry->size = io_read_u32_le(io);
        if (entry->offset + entry->size > io_size(io))
        {
            log_error("Bad offset to file");
            free(entry);
            free(entry->name);
            return false;
        }
        entry->io = io;
        old_pos = io_tell(io);
        output_files_save(output_files, &mbl_read_file, (void*)entry);
        io_seek(io, old_pos);
        free(entry->name);
        free(entry);
    }

    return true;
}

void mbl_cleanup(Archive *archive)
{
    assert_not_null(archive);
    converter_destroy((Converter*)archive->data);
}

Archive *mbl_archive_create()
{
    Archive *archive = archive_create();
    archive->unpack = &mbl_unpack;
    archive->cleanup = &mbl_cleanup;
    archive->data = (void*)prs_converter_create();
    assert_not_null(archive->data);
    return archive;
}
