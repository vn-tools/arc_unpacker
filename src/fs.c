#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "fs.h"
#include "logger.h"
#include "string_ext.h"

bool mkpath(const char *path)
{
    struct stat sb;
    const char *slash;
    bool done = false;
    char *dir_path = NULL;

    slash = path;
    while (!done)
    {
        slash += strcspn(slash, "/\\");
        slash += strspn(slash, "/\\");
        done = (*slash == '\0');

        dir_path = malloc(slash + 2 - path);
        if (!dir_path)
        {
            log_error(NULL);
            return false;
        }
        memset(dir_path, 0, slash + 2 - path);
        strncpy(dir_path, path, slash - path);
        trim_right(dir_path, "/\\");
        strcat(dir_path, "/"); //important for Windows

        if (stat(dir_path, &sb))
        {
            if (errno != ENOENT)
            {
                log_warning("Failed to stat path %s", dir_path);
                free(dir_path);
                return false;
            }
            if (mkdir(dir_path, 0755) != 0)
            {
                log_warning("Failed to create directory %s", dir_path);
                free(dir_path);
                return false;
            }
        }
        else if (!S_ISDIR(sb.st_mode))
        {
            errno = EEXIST;
            log_warning("Failed to create directory %s", dir_path);
            free(dir_path);
            return false;
        }
        free(dir_path);
    }

    return true;
}
