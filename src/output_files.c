#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "fs.h"
#include "logger.h"
#include "output_files.h"

struct OutputFiles
{
    const char *output_dir;
};

OutputFiles *output_files_create(const char *output_dir)
{
    OutputFiles *output_files = (OutputFiles*)malloc(sizeof(OutputFiles));
    assert_not_null(output_files);
    output_files->output_dir = output_dir;
    return output_files;
}

void output_files_destroy(OutputFiles *output_files)
{
    assert_not_null(output_files);
    free(output_files);
}

bool output_files_save(
    OutputFiles *output_files,
    VirtualFile *(*save_proc)(void *),
    void *context)
{
    VirtualFile *file;
    char *full_path;
    char *dir;
    FILE *fp;
    bool verbose = log_enabled(LOG_LEVEL_INFO);

    assert_not_null(output_files);
    assert_not_null(output_files->output_dir);
    assert_that(save_proc != NULL);

    if (verbose)
    {
        printf("Extracting... ");
        fflush(stdout);
    }

    file = save_proc(context);
    if (file != NULL)
    {

        full_path = (char*)malloc(
            strlen(output_files->output_dir)
            + 1
            + strlen(vf_get_name(file)) + 1);
        assert_not_null(full_path);
        strcpy(full_path, output_files->output_dir);
        strcat(full_path, "/");
        strcat(full_path, vf_get_name(file));

        if (verbose)
        {
            printf("Saving to %s... ", full_path);
            fflush(stdout);
        }

        dir = dirname(full_path);
        assert_not_null(dir);
        assert_that(mkpath(dirname(full_path)));
        fp = fopen(full_path, "wb");
        if (!fp)
        {
            log_warning("Failed to open file %s", full_path);
        }
        else
        {
            fwrite(vf_get_data(file), 1, vf_get_size(file), fp);
            fclose(fp);
            if (verbose)
                puts("ok");
            vf_destroy(file);
            free(full_path);
            free(dir);
            return true;
        }
        free(full_path);
        free(dir);
    }

    //errors already reported with log_error and log_warn, no need to print \n
    vf_destroy(file);
    return false;
}
