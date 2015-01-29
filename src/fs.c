#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "fs.h"
#include "logger.h"

bool mkpath(const char *path)
{
    struct stat sb;
    char *slash;
    bool done = false;

    slash = (char*)path;

    while (!done)
    {
        slash += strspn(slash, "/");
        slash += strcspn(slash, "/");

        done = (*slash == '\0');

        *slash = '\0';
        if (stat(path, &sb))
        {
            if (errno != ENOENT)
            {
                *slash = '/';
                log_warning("Failed to stat path %s", path);
                return false;
            }
            if (mkdir(path, 0755) != 0)
            {
                *slash = '/';
                log_warning("Failed to create directory %s", path);
                return false;
            }
        }
        else if (!S_ISDIR(sb.st_mode))
        {
            *slash = '/';
            errno = EEXIST;
            log_warning("Failed to create directory %s", path);
            return false;
        }

        *slash = '/';
    }

    return true;
}
