#include <set>
#include "file_io.h"
#include "file_saver.h"
#include "fs.h"
#include "logger.h"
#include "util/itos.h"

struct FileSaverHdd::Internals
{
    std::string output_dir;
    std::set<std::string> paths;

    std::string get_full_path(
        const std::string output_dir, const std::string file_name)
    {
        if (output_dir == "")
            return file_name;
        std::string path = output_dir;
        if (path[path.length() - 1] != '/')
            path += "/";
        path += file_name;
        return path;
    }

    std::string make_path_unique(const std::string path)
    {
        std::string full_path = path;
        int i = 1;
        while (paths.find(full_path) != paths.end())
        {
            std::string dir = dirname(path);
            std::string name = basename(path);
            std::string suffix = "(" + itos(i ++) + ")";
            size_t pos = name.rfind(".");
            name = pos != std::string::npos
                ? name.substr(0, pos) + suffix + name.substr(pos)
                : name + suffix;
            full_path = dir + "/" + name;
        }
        paths.insert(full_path);
        return full_path;
    }
};

FileSaverHdd::FileSaverHdd(std::string output_dir)
    : internals(new Internals)
{
    internals->output_dir = output_dir;
}

FileSaverHdd::~FileSaverHdd()
{
}

void FileSaverHdd::save(const std::shared_ptr<File> &file) const
{
    try
    {
        std::string full_path = internals->get_full_path(
            internals->output_dir, file->name);
        full_path = internals->make_path_unique(full_path);

        log("Saving to %s... ", full_path.c_str());

        mkpath(dirname(full_path));

        FileIO output_io(full_path, "wb");
        file->io.seek(0);
        output_io.write_from_io(file->io, file->io.size());
        log("ok\n");
    }
    catch (std::runtime_error &e)
    {
        log("Error (%s)\n", e.what());
    }
}



struct FileSaverMemory::Internals
{
    std::vector<std::shared_ptr<File>> files;
};

FileSaverMemory::FileSaverMemory() : internals(new Internals)
{
}

FileSaverMemory::~FileSaverMemory()
{
}

const std::vector<std::shared_ptr<File>> FileSaverMemory::get_saved() const
{
    return internals->files;
}

void FileSaverMemory::save(const std::shared_ptr<File> &file) const
{
    internals->files.push_back(file);
}
