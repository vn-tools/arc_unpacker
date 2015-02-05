#include <cassert>
#include <cerrno>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "fs.h"
#include "logger.h"

#if defined(__linux) || defined(__unix)
    #define p_mkdir(name,chmod) mkdir(name, chmod)
#else
    #define p_mkdir(name,chmod) _mkdir(name)
#endif

namespace
{
    std::string trim_right(std::string str, std::string chars)
    {
        size_t pos = str.length();
        while (pos > 0 && chars.find(str[pos - 1]) != std::string::npos)
            -- pos;
        return str.substr(0, pos);
    }

    size_t find_last_slash(const std::string path)
    {
        size_t pos = path.length();
        while (pos > 0)
        {
            -- pos;
            if (path[pos] == '\\' || path[pos] == '/')
                return pos;
        }
        return std::string::npos;
    }

    bool get_files_accumulator(
        const std::string dir_path,
        std::vector<std::string> &accumulator,
        bool recursive)
    {
        DIR *d = opendir(dir_path.c_str());
        if (!d)
        {
            log_warning("FS: Cannot open directory %s", dir_path.c_str());
            return false;
        }

        while (1)
        {
            struct dirent *entry = readdir(d);
            if (!entry)
                break;

            std::string name = std::string(entry->d_name);
            std::string path = dir_path + "/" + name;
            if (is_dir(path))
            {
                if (recursive && name != ".." && name != ".")
                {
                    get_files_accumulator(path, accumulator, recursive);
                }
            }
            else
            {
                accumulator.push_back(path);
            }
        }

        closedir(d);
        return true;
    }
}

bool is_dir(const std::string path)
{
    struct stat stats;
    return stat(path.c_str(), &stats) == 0 && S_ISDIR (stats.st_mode);
}

std::vector<std::string> get_files_recursive(const std::string dir_path)
{
    std::vector<std::string> accumulator;
    get_files_accumulator(dir_path, accumulator, true);
    return accumulator;
}

std::vector<std::string> get_files(const std::string dir_path)
{
    std::vector<std::string> accumulator;
    get_files_accumulator(dir_path, accumulator, false);
    return accumulator;
}

std::string basename(const std::string path)
{
    size_t last_slash = find_last_slash(path);
    if (last_slash == std::string::npos)
        return path;
    return path.substr(last_slash + 1);
}

std::string dirname(const std::string path)
{
    std::string path_nts = trim_right(path, "/\\");
    if (path_nts == "")
        return path;

    size_t last_slash = find_last_slash(path_nts);
    if (last_slash == std::string::npos)
        return path;
    return path_nts.substr(0, last_slash + 1);
}

bool mkpath(const std::string path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb))
    {
        if (errno != ENOENT)
        {
            log_warning("FS: Failed to stat path %s", path.c_str());
            return false;
        }
        std::string dir = dirname(path);
        if (!mkpath(dir))
            return false;
        trim_right(dir, "/\\");
        if (p_mkdir(path.c_str(), 0755) != 0 && errno != EEXIST)
        {
            log_warning("FS: Failed to create directory %s", dir.c_str());
            return false;
        }
        return true;
    }
    else if (!S_ISDIR(sb.st_mode))
    {
        log_warning(
            "FS: Cannot create directory at %s - file already exists",
            path.c_str());
        return false;
    }
    return true;
}
