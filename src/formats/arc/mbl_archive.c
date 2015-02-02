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
    IO *arc_file;

    char *name;
    uint32_t offset;
    uint32_t size;
} TableEntry;

static int mbl_check_version(
    IO *arc_file,
    size_t initial_position,
    uint32_t file_count,
    uint32_t name_length)
{
    io_seek(arc_file, initial_position + file_count * (name_length + 8));
    io_skip(arc_file, -8);
    uint32_t last_file_offset = io_read_u32_le(arc_file);
    uint32_t last_file_size = io_read_u32_le(arc_file);
    return last_file_offset + last_file_size == io_size(arc_file);
}

static int mbl_get_version(IO *arc_file)
{
    uint32_t file_count = io_read_u32_le(arc_file);
    if (mbl_check_version(arc_file, 4, file_count, 16))
    {
        io_seek(arc_file, 0);
        return 1;
    }

    io_seek(arc_file, 4);
    uint32_t name_length = io_read_u32_le(arc_file);
    if (mbl_check_version(arc_file, 8, file_count, name_length))
    {
        io_seek(arc_file, 0);
        return 2;
    }

    return -1;
}

static VirtualFile *mbl_read_file(void *context)
{
    VirtualFile *vf = vf_create();
    TableEntry *table_entry = (TableEntry*)context;
    io_seek(table_entry->arc_file, table_entry->offset);
    io_write_string_from_io(vf->io, table_entry->arc_file, table_entry->size);
    vf_set_name(vf, table_entry->name);

    converter_try_decode((Converter*)table_entry->archive->data, vf);

    return vf;
}

static bool mbl_unpack(
    Archive *archive,
    IO *arc_file,
    OutputFiles *output_files)
{
    size_t i, old_pos;
    int version = mbl_get_version(arc_file);
    if (version == -1)
    {
        log_error("Not a MBL archive");
        return false;
    }
    log_info("Version: %d", version);

    uint32_t file_count = io_read_u32_le(arc_file);
    uint32_t name_length = version == 2 ? io_read_u32_le(arc_file) : 16;
    for (i = 0; i < file_count; i ++)
    {
        TableEntry *entry = (TableEntry*)malloc(sizeof(TableEntry));
        assert_not_null(entry);

        old_pos = io_tell(arc_file);
        char *tmp_name = NULL;
        io_read_until_zero(arc_file, &tmp_name, NULL);
        assert_not_null(tmp_name);
        assert_that(convert_encoding(
            tmp_name, strlen(tmp_name),
            &entry->name, NULL,
            "sjis", "utf-8"));
        free(tmp_name);
        io_seek(arc_file, old_pos + name_length);

        entry->archive = archive;
        entry->offset = io_read_u32_le(arc_file);
        entry->size = io_read_u32_le(arc_file);
        if (entry->offset + entry->size > io_size(arc_file))
        {
            log_error("Bad offset to file");
            free(entry);
            free(entry->name);
            return false;
        }
        entry->arc_file = arc_file;
        old_pos = io_tell(arc_file);
        output_files_save(output_files, &mbl_read_file, (void*)entry);
        io_seek(arc_file, old_pos);
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
