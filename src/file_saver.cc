#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <set>
#include <stdexcept>
#include "file_saver.h"
#include "io/file_io.h"
#include "util/itos.h"

struct FileSaverHdd::Priv
{
    boost::filesystem::path output_dir;
    std::set<boost::filesystem::path> paths;
    bool overwrite;

    Priv(const boost::filesystem::path &output_dir, bool overwrite)
         : output_dir(output_dir), overwrite(overwrite)
    {
    }

    boost::filesystem::path make_path_unique(const boost::filesystem::path path)
    {
        boost::filesystem::path new_path = path;
        int i = 1;
        while (paths.find(new_path) != paths.end()
        || (!overwrite && boost::filesystem::exists(new_path)))
        {
            std::string suffix = "(" + itos(i++) + ")";
            new_path = path.parent_path();
            new_path /= boost::filesystem::path(
                path.stem().string() + suffix + path.extension().string());
        }
        paths.insert(new_path);
        return new_path;
    }
};

FileSaverHdd::FileSaverHdd(
    const boost::filesystem::path &output_dir, bool overwrite)
    : p(new Priv(output_dir, overwrite))
{
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
            pos++;
        }

        boost::filesystem::path full_path(p->output_dir);
        full_path /= boost::filesystem::path(name_part);
        full_path = p->make_path_unique(full_path);

        std::cout << "Saving to " << full_path.generic_string() << "... ";

        if (full_path.parent_path() != "")
            boost::filesystem::create_directories(full_path.parent_path());

        FileIO output_io(full_path.string(), FileIOMode::Write);
        file->io.seek(0);
        output_io.write_from_io(file->io, file->io.size());
        std::cout << "ok\n";
    }
    catch (std::runtime_error &e)
    {
        std::cout << "error (" << e.what() << ")\n";
    }
    std::cout.flush();
}



struct FileSaverCallback::Priv
{
    FileSaveCallback callback;

    Priv(FileSaveCallback callback) : callback(callback)
    {
    }
};

FileSaverCallback::FileSaverCallback()
    : p(new Priv(nullptr))
{
}

FileSaverCallback::FileSaverCallback(FileSaveCallback callback)
    : p(new Priv(callback))
{
}

FileSaverCallback::~FileSaverCallback()
{
}

void FileSaverCallback::set_callback(FileSaveCallback callback)
{
    p->callback = callback;
}

void FileSaverCallback::save(std::shared_ptr<File> file) const
{
    p->callback(file);
}
