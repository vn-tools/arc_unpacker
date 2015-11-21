#include "util/program_path.h"
#include "io/filesystem.h"
#include "err.h"

using namespace au;

namespace
{
    static io::path program_path;
}

void util::set_program_path_from_arg(const std::string &arg)
{
    io::path full_path(io::current_working_directory());
    program_path = io::complete(io::path(arg)).str();
}

io::path util::get_program_path()
{
    return program_path;
}

io::path util::get_extra_dir_path()
{
    const io::path path(program_path.parent());
    const io::path path1 = path / "extra";
    if (io::is_directory(path1))
        return path1.str();
    const io::path path2 = path.parent() / "extra";
    if (io::is_directory(path2))
        return path2;
    throw err::FileNotFoundError("Can't locate 'extra/' directory!");
}
