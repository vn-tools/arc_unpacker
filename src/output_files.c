#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "fs.h"
#include "logger.h"
#include "output_files.h"

struct OutputFiles
{
    const char *output_dir;
    bool (*save)(OutputFiles*, VirtualFile*(*)(void *), void *);
    bool memory;
    Array *files;
};

static char *get_full_path(OutputFiles *output_files, const char *file_name)
{
    assert_not_null(output_files);
    if (output_files->output_dir == NULL)
        return strdup(file_name);

    char *full_path = (char*)malloc(
        strlen(output_files->output_dir)
        + 1
        + strlen(file_name) + 1);
    assert_not_null(full_path);
    strcpy(full_path, output_files->output_dir);
    strcat(full_path, "/");
    strcat(full_path, file_name);
    return full_path;
}

static bool save_to_hdd(
    OutputFiles *output_files,
    VirtualFile *(*save_proc)(void *),
    void *context)
{
    assert_not_null(output_files);
    assert_that(!output_files->memory);
    assert_that(save_proc != NULL);

    log_info("Reading file...");

    bool result;
    VirtualFile *file = save_proc(context);
    if (file != NULL)
    {
        char *full_path = get_full_path(output_files, vf_get_name(file));
        assert_not_null(full_path);

        log_info("Saving to %s... ", full_path);

        char *dir = dirname(full_path);
        assert_not_null(dir);
        assert_that(mkpath(dirname(full_path)));
        FILE *fp = fopen(full_path, "wb");
        if (!fp)
        {
            log_warning("Failed to open file %s", full_path);
        }
        else
        {
            fwrite(vf_get_data(file), 1, vf_get_size(file), fp);
            fclose(fp);
            result = true;
            log_info("Saved successfully");
        }
        free(full_path);
        free(dir);
    }
    else
    {
        log_error("Error while reading file");
    }

    if (file != NULL)
        vf_destroy(file);
    return result;
}

static bool save_to_memory(
    OutputFiles *output_files,
    VirtualFile *(*save_proc)(void *),
    void *context)
{
    VirtualFile *vf;
    assert_not_null(output_files);
    assert_that(save_proc != NULL);
    assert_that(output_files->memory);
    vf = save_proc(context);
    if (vf == NULL)
    {
        return false;
    }
    assert_that(array_set(
        output_files->files,
        array_size(output_files->files),
        vf));
    return true;
}

OutputFiles *output_files_create_hdd(const char *output_dir)
{
    OutputFiles *output_files = (OutputFiles*)malloc(sizeof(OutputFiles));
    assert_not_null(output_files);
    output_files->output_dir = output_dir;
    output_files->memory = false;
    output_files->files = NULL;
    output_files->save = &save_to_hdd;
    return output_files;
}

OutputFiles *output_files_create_memory()
{
    OutputFiles *output_files = (OutputFiles*)malloc(sizeof(OutputFiles));
    assert_not_null(output_files);
    output_files->output_dir = NULL;
    output_files->memory = true;
    output_files->files = array_create();
    assert_that(output_files->files);
    output_files->save = &save_to_memory;
    return output_files;
}

void output_files_destroy(OutputFiles *output_files)
{
    size_t i;
    assert_not_null(output_files);
    if (output_files->memory)
    {
        for (i = 0; i < array_size(output_files->files); i ++)
            vf_destroy((VirtualFile*)array_get(output_files->files, i));
        array_destroy(output_files->files);
    }
    free(output_files);
}

bool output_files_save(
    OutputFiles *output_files,
    VirtualFile *(*save_proc)(void *),
    void *context)
{
    assert_not_null(output_files);
    assert_that(output_files->save != NULL);
    return output_files->save(output_files, save_proc, context);
}

Array *output_files_get_saved(const OutputFiles *output_files)
{
    assert_not_null(output_files);
    assert_that(output_files->memory);
    return output_files->files;
}
