#include <set>
#include "file_io.h"
#include "fs.h"
#include "logger.h"
#include "output_files.h"
#include "string_ex.h"

void OutputFiles::save(VFFactory save_proc) const
{
    save([&]()
    {
        std::vector<std::unique_ptr<VirtualFile>> single_item;
        std::unique_ptr<VirtualFile> file = save_proc();
        single_item.push_back(std::move(file));
        return single_item;
    });
}



struct OutputFilesHdd::Internals
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
            std::string suffix = "(" + stoi(i ++) + ")";
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

OutputFilesHdd::OutputFilesHdd(std::string output_dir)
    : internals(new Internals)
{
    internals->output_dir = output_dir;
}

OutputFilesHdd::~OutputFilesHdd()
{
}

void OutputFilesHdd::save(VFsFactory save_proc) const
{
    try
    {
        for (auto &file : save_proc())
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

    }
    catch (std::runtime_error &e)
    {
        log("Error (%s)\n", e.what());
    }
}



struct OutputFilesMemory::Internals
{
    std::vector<std::unique_ptr<VirtualFile>> files;
};

OutputFilesMemory::OutputFilesMemory() : internals(new Internals)
{
}

OutputFilesMemory::~OutputFilesMemory()
{
}

const std::vector<VirtualFile*> OutputFilesMemory::get_saved() const
{
    std::vector<VirtualFile*> files;
    for (std::unique_ptr<VirtualFile>& f : internals->files)
        files.push_back(f.get());
    return files;
}

void OutputFilesMemory::save(VFsFactory save_proc) const
{
    for (auto &file : save_proc())
        internals->files.push_back(std::move(file));
}
