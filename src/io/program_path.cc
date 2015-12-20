#include "io/program_path.h"
#include "err.h"
#include "io/file_system.h"

using namespace au;

namespace
{
    static io::path program_path;
}

void io::set_program_path_from_arg(const std::string &arg)
{
    program_path = io::absolute(io::path(arg)).str();
}

io::path io::get_program_path()
{
    return program_path;
}

io::path io::get_assets_dir_path()
{
    auto dir = program_path.parent();
    do
    {
        const auto path = dir / "etc";
        if (io::is_directory(path))
            return path.str();
        dir = dir.parent();
    }
    while (!dir.is_root());
    throw err::FileNotFoundError("Can't locate 'etc/' assets directory!");
}
