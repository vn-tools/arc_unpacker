#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "errno.h"
#include "output_files.h"
#include "fs.h"

static bool is_quiet(const Options *const options);

static bool is_quiet(const Options *const options)
{
    const char *verbosity = options_get(options, "verbosity");
    assert_not_null(verbosity);
    return strcmp(verbosity, "quiet") == 0;
}



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
    bool quiet;
    const char *output_dir;
    char *full_path;
    FILE *fp;

    assert_not_null(output_files);
    assert_that(save_proc != NULL);
    quiet = is_quiet(output_files->options);

    if (!quiet)
        printf("Extracting... ");

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
            if (!quiet)
                printf("Saving to %s... ", full_path);
            if (mkpath(output_dir))
            {
                fp = fopen(full_path, "wb");
                if (!fp)
                {
                    errno = EIO;
                    warn("Failed to open file %s: %s\n", full_path, strerror(errno));
                }
                else
                {
                    fwrite(vf_get_data(file), 1, vf_get_size(file), fp);
                    fclose(fp);
                    if (!quiet)
                        printf("ok\n");
                    vf_destroy(file);
                    return true;
                }
            }
        }
    }

    printf("error (%s)\n", strerror(errno));
    vf_destroy(file);
    return false;
}
