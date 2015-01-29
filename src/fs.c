#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "assert.h"
#include "fs.h"
#include "logger.h"
#include "string_ex.h"

char *dirname(const char *path)
{
    char *ret;
    char *path_nts;

    path_nts = strdup(path);
    assert_not_null(path_nts);
    trim_right(path_nts, "/\\");
    if (strcmp(path_nts, "") == 0)
    {
        free(path_nts);
        return strdup(path);
    }

    char *last_slash = strrchr(path_nts, '\\');
    if (strrchr(path_nts, '/') > last_slash)
        last_slash = strrchr(path_nts, '/');
    if (last_slash == NULL)
    {
        free(path_nts);
        return strdup(path);
    }

    ret = strndup(path_nts, last_slash + 1 - path_nts);
    free(path_nts);
    return ret;
}

bool mkpath(const char *path)
{
    errno = 0;
    char *dir = NULL;
    struct stat sb;
    if (stat(path, &sb))
    {
        if (errno != ENOENT)
        {
            log_warning("Failed to stat path %s", path);
            return false;
        }
        dir = dirname(path);
        if (!mkpath(dir))
        {
            free(dir);
            return false;
        }
        trim_right(dir, "/\\");
        if (mkdir(path, 0755) != 0 && errno != EEXIST)
        {
            log_warning("Failed to create directory %s", dir);
            free(dir);
            return false;
        }
        free(dir);
        return true;
    }
    else if (!S_ISDIR(sb.st_mode))
    {
        log_warning("Cannot create directory at %s - file already exists", path);
        return false;
    }
    return true;
}
