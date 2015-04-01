#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <set>
#include <stdexcept>
#include "file_io.h"
#include "file_saver.h"
#include "logger.h"
#include "util/itos.h"

struct FileSaverHdd::Internals
{
    boost::filesystem::path output_dir;
    std::set<boost::filesystem::path> paths;

    boost::filesystem::path make_path_unique(const boost::filesystem::path path)
    {
        boost::filesystem::path new_path = path;
        int i = 1;
        while (paths.find(new_path) != paths.end())
        {
            std::string suffix = "(" + itos(i ++) + ")";
            new_path = path.parent_path();
            new_path /= boost::filesystem::path(
                path.stem().string() + suffix + path.extension().string());
        }
        paths.insert(new_path);
        return new_path;
    }
};

FileSaverHdd::FileSaverHdd(boost::filesystem::path output_dir)
    : internals(new Internals)
{
    internals->output_dir = output_dir;
}

FileSaverHdd::~FileSaverHdd()
{
}

void FileSaverHdd::save(std::shared_ptr<File> file) const
{
    try
    {
        std::string name_part = file->name;
        size_t pos = 0;
        while ((pos = name_part.find("\\", pos)) != std::string::npos)
        {
            name_part.replace(pos, 1, "/");
            pos ++;
        }

        boost::filesystem::path full_path(internals->output_dir);
        full_path /= boost::filesystem::path(name_part);
        full_path = internals->make_path_unique(full_path);

        log("Saving to %s... ", full_path.generic_string().c_str());

        boost::filesystem::create_directories(full_path.parent_path());

        FileIO output_io(full_path.string(), FileIOMode::Write);
        file->io.seek(0);
        output_io.write_from_io(file->io, file->io.size());
        log("ok\n");
    }
    catch (std::runtime_error &e)
    {
        log("Error (%s)\n", e.what());
    }
}



struct FileSaverCallback::Internals
{
    FileSaveCallback callback;

    Internals(FileSaveCallback callback) : callback(callback)
    {
    }
};

FileSaverCallback::FileSaverCallback(FileSaveCallback callback)
    : internals(new Internals(callback))
{
}

FileSaverCallback::~FileSaverCallback()
{
}

void FileSaverCallback::save(std::shared_ptr<File> file) const
{
    internals->callback(file);
}
