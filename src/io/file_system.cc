#include "io/file_system.h"
#include <boost/filesystem/path.hpp>

using namespace au;
using namespace au::io;

bool io::exists(const path &p)
{
    return boost::filesystem::exists(p.str());
}

bool io::is_regular_file(const path &p)
{
    return boost::filesystem::is_regular_file(p.str());
}

bool io::is_directory(const path &p)
{
    return boost::filesystem::is_directory(p.str());
}

path io::current_working_directory()
{
    return boost::filesystem::current_path().string();
}

path io::absolute(const path &p)
{
    return boost::filesystem::absolute(p.str()).string();
}

void io::create_directories(const path &p)
{
    const auto bp = boost::filesystem::path(p.str());
    if (!bp.empty())
        boost::filesystem::create_directories(bp);
}

void io::remove(const path &p)
{
    boost::filesystem::remove(p.str());
}
