#include <cassert>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "fs.h"
#include "logger.h"
#include "string_ex.h"

#if defined(__linux) || defined(__unix)
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
        log_warning("FS: Cannot open directory %s", dir_path);
        return false;
    }

    while (1)
    {
        struct dirent *entry = readdir(d);
        if (!entry)
            break;

        int path_length = strlen(dir_path) + 1 + strlen(entry->d_name);
        char *path = new char[path_length + 1];
        assert(path != nullptr);

        strcpy(path, dir_path);
        strcat(path, "/");
        strcat(path, entry->d_name);

        if (is_dir(path))
        {
            if (recursive
            && strcmp(entry->d_name, "..") != 0
            && strcmp(entry->d_name, ".") != 0)
            {
                _get_files_accumulator(path, accumulator, recursive);
            }
            delete []path;
        }
        else
        {
            array_add(accumulator, path);
        }
    }

    closedir(d);
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
    return stat(path, &stats) == 0 && S_ISDIR (stats.st_mode);
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
    if (last_slash == nullptr)
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
    assert(path_nts != nullptr);
    trim_right(path_nts, "/\\");
    if (strcmp(path_nts, "") == 0)
    {
        delete []path_nts;
        return strdup(path);
    }

    char *last_slash = strrchr(path_nts, '\\');
    if (strrchr(path_nts, '/') > last_slash)
        last_slash = strrchr(path_nts, '/');
    if (last_slash == nullptr)
    {
        delete []path_nts;
        return strdup(path);
    }

    ret = strndup(path_nts, last_slash + 1 - path_nts);
    delete []path_nts;
    return ret;
}

bool mkpath(const char *path)
{
    char *dir = nullptr;
    struct stat sb;
    if (stat(path, &sb))
    {
        if (errno != ENOENT)
        {
            log_warning("FS: Failed to stat path %s", path);
            return false;
        }
        dir = dirname(path);
        if (!mkpath(dir))
        {
            delete []dir;
            return false;
        }
        trim_right(dir, "/\\");
        if (p_mkdir(path, 0755) != 0 && errno != EEXIST)
        {
            log_warning("FS: Failed to create directory %s", dir);
            delete []dir;
            return false;
        }
        delete []dir;
        return true;
    }
    else if (!S_ISDIR(sb.st_mode))
    {
        log_warning(
            "FS: Cannot create directory at %s - file already exists",
            path);
        return false;
    }
    return true;
}
