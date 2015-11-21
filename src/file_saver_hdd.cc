#include "file_saver.h"
#include <boost/algorithm/string/replace.hpp>
#include <set>
#include "io/file_stream.h"
#include "io/filesystem.h"
#include "log.h"
#include "util/format.h"

using namespace au;

struct FileSaverHdd::Priv final
{
    Priv(const io::path &output_dir, bool overwrite);
    io::path make_path_unique(const io::path &path);

    io::path output_dir;
    std::set<io::path> paths;
    bool overwrite;
};

FileSaverHdd::Priv::Priv(const io::path &output_dir, bool overwrite)
        : output_dir(output_dir), overwrite(overwrite)
{
}

io::path FileSaverHdd::Priv::make_path_unique(const io::path &path)
{
    io::path new_path = path;
    int i = 1;
    while (paths.find(new_path) != paths.end()
        || (!overwrite && io::exists(new_path)))
    {
        const auto suffix = util::format("(%d)", i++);
        new_path = path.parent();
        new_path /= io::path(path.stem() + suffix + path.extension());
    }
    paths.insert(new_path);
    return new_path;
}

FileSaverHdd::FileSaverHdd(const io::path &output_dir, bool overwrite)
    : p(new Priv(output_dir, overwrite))
{
}

FileSaverHdd::~FileSaverHdd()
{
}

void FileSaverHdd::save(std::shared_ptr<io::File> file) const
{
    try
    {
        auto name_part = file->name;
        size_t pos = 0;
        while ((pos = name_part.find("\\", pos)) != std::string::npos)
        {
            name_part.replace(pos, 1, "/");
            pos++;
        }

        io::path full_path(p->output_dir);
        full_path /= io::path(name_part);
        full_path = p->make_path_unique(full_path);

        Log.info("Saving to " + full_path.str() + "... ");
        io::create_directories(full_path.parent());
        io::FileStream output_stream(full_path, io::FileMode::Write);
        file->stream.seek(0);
        output_stream.write(file->stream.read_to_eof());
        Log.success("ok\n");
    }
    catch (std::runtime_error &e)
    {
        Log.err("error (%s)\n", e.what() ? e.what() : "unknown error");
    }
    Log.flush();
}
