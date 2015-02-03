#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "assert_ex.h"
#include "fs.h"
#include "logger.h"
#include "string_ex.h"

#if defined(linux) || defined(unix)
    #define p_mkdir(name,chmod) mkdir(name, chmod)
#else
    #define p_mkdir(name,chmod) _mkdir(name)
#endif

static bool _get_files_accumulator(
    const char *dir_path,
    Array *accumulator,
    bool recursive)
{
    DIR *d = opendir(dir_path);
    if (!d)
    {
        log_warning("Cannot open directory %s", dir_path);
        return false;
    }

    while (1)
    {
        struct dirent *entry = readdir(d);
        if (!entry)
            break;

        int path_length = strlen(dir_path) + 1 + strlen(entry->d_name);
        char *path = (char*)malloc(path_length + 1);
        assert_not_null(path);

        strcpy(path, dir_path);
        strcat(path, "/");
        strcat(path, entry->d_name);

        if (entry->d_type & DT_DIR)
        {
            if (recursive
            && strcmp(entry->d_name, "..") != 0
            && strcmp(entry->d_name, ".") != 0)
            {
                _get_files_accumulator(path, accumulator, recursive);
            }
            free(path);
        }
        else
        {
            array_add(accumulator, path);
        }
    }

    assert_equali(0, closedir(d));
    return true;
}

static Array *_get_files(const char *dir_path, bool recursive)
{
    Array *accumulator = array_create();
    if (!_get_files_accumulator(dir_path, accumulator, recursive))
    {
        array_destroy(accumulator);
        return false;
    }
    return accumulator;
}



bool is_dir(const char *path)
{
    struct stat stats;
    return stat (path, &stats) == 0 && S_ISDIR (stats.st_mode);
}

Array *get_files_recursive(const char *dir_path)
{
    return _get_files(dir_path, true);
}

Array *get_files(const char *dir_path)
{
    return _get_files(dir_path, false);
}

char *basename(const char *path)
{
    char *last_slash = strrchr(path, '\\');
    if (strrchr(path, '/') > last_slash)
        last_slash = strrchr(path, '/');
    if (last_slash == NULL)
    {
        return strdup(path);
    }
    return strdup(last_slash + 1);
}

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
        if (p_mkdir(path, 0755) != 0 && errno != EEXIST)
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
        log_warning("Cannot create directory at %s: file already exists", path);
        return false;
    }
    return true;
}
