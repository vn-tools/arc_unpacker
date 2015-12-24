#include "file_saver.h"
#include <mutex>
#include <set>
#include "algo/format.h"
#include "io/file_stream.h"
#include "io/file_system.h"
#include "log.h"

using namespace au;

static std::mutex mutex;

struct FileSaverHdd::Priv final
{
    Priv(const io::path &output_dir, const bool overwrite);
    io::path make_path_unique(const io::path &path);

    io::path output_dir;
    std::set<io::path> paths;
    bool overwrite;
};

FileSaverHdd::Priv::Priv(const io::path &output_dir, const bool overwrite)
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
        new_path.change_stem(path.stem() + algo::format("(%d)", i++));
    }
    paths.insert(new_path);
    return new_path;
}

FileSaverHdd::FileSaverHdd(const io::path &output_dir, const bool overwrite)
    : p(new Priv(output_dir, overwrite))
{
}

FileSaverHdd::~FileSaverHdd()
{
}

void FileSaverHdd::save(std::shared_ptr<io::File> file) const
{
    std::unique_lock<std::mutex> lock(mutex);
    try
    {
        const auto full_path = p->make_path_unique(p->output_dir / file->path);

        Log.info("Saving to " + full_path.str() + "... ");
        io::create_directories(full_path.parent());
        io::FileStream output_stream(full_path, io::FileMode::Write);
        output_stream.write(file->stream.seek(0).read_to_eof());
        Log.success("ok\n");
    }
    catch (std::runtime_error &e)
    {
        Log.err("error (%s)\n", e.what() ? e.what() : "unknown error");
    }
    Log.flush();
}
