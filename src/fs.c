#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include "fs.h"

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
                warn("Failed to stat path %s: %s\n", path, strerror(errno));
                return false;
            }
            if (mkdir(path, 0755) != 0)
            {
                *slash = '/';
                warn("Failed to create directory %s: %s\n", path, strerror(errno));
                return false;
            }
        }
        else if (!S_ISDIR(sb.st_mode))
        {
            *slash = '/';
            errno = EEXIST;
            warn("Failed to create directory %s: %s\n", path, strerror(errno));
            return false;
        }

        *slash = '/';
    }

    return true;
}
