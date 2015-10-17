#include "util/program_path.h"
#include <boost/filesystem.hpp>
#include "err.h"

using namespace au;
namespace fs = boost::filesystem;

namespace
{
    static fs::path program_path;
}

void util::set_program_path_from_arg(const std::string &arg)
{
    fs::path full_path(fs::initial_path<fs::path>());
    program_path = fs::system_complete(fs::path(arg)).string();
}

fs::path util::get_program_path()
{
    return program_path;
}

fs::path util::get_extra_dir_path()
{
    const fs::path path(program_path.parent_path());
    const fs::path path1 = path / "extra";
    if (fs::is_directory(path1))
        return path1.string();
    const fs::path path2 = path.parent_path() / "extra";
    if (fs::is_directory(path2))
        return path2;
    throw err::FileNotFoundError("Can't locate 'extra/' directory!");
}
