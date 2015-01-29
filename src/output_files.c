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
    const Options *options;
};

OutputFiles *output_files_create(const Options *const options)
{
    OutputFiles *output_files = malloc(sizeof(OutputFiles));
    output_files->options = options;
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
    const char *output_dir;
    char *full_path;
    FILE *fp;
    bool verbose = log_enabled(LOG_LEVEL_INFO);

    assert_not_null(output_files);
    assert_that(save_proc != NULL);

    if (verbose)
    {
        printf("Extracting... ");
        fflush(stdout);
    }

    file = save_proc(context);
    if (file != NULL)
    {
        output_dir = options_get(output_files->options, "output_path");
        assert_not_null(output_dir);

        full_path = malloc(strlen(output_dir) + 1 + strlen(vf_get_name(file)) + 1);
        if (!full_path)
        {
            errno = ENOMEM;
        }
        else
        {
            strcpy(full_path, output_dir);
            strcat(full_path, "/");
            strcat(full_path, vf_get_name(file));
            if (verbose)
            {
                printf("Saving to %s... ", full_path);
                fflush(stdout);
            }
            if (mkpath(output_dir))
            {
                fp = fopen(full_path, "wb");
                if (!fp)
                {
                    errno = EIO;
                    log_warning("Failed to open file %s", full_path);
                }
                else
                {
                    fwrite(vf_get_data(file), 1, vf_get_size(file), fp);
                    fclose(fp);
                    if (verbose)
                        puts("ok");
                    vf_destroy(file);
                    return true;
                }
            }
        }
    }

    //errors already reported with log_error and log_warn, no need to print \n
    vf_destroy(file);
    return false;
}
