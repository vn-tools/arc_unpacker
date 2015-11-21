#include "io/path.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>

using namespace au;
using namespace au::io;

static std::string normalize_extension(const std::string &extension)
{
    std::string output(extension);
    while (!output.empty() && output[0] == '.')
        output.erase(0, 1);
    if (!output.empty())
        output = "." + output;
    return output;
}

path::path() : p("")
{
}

path::path(const char *s) : p(s)
{
}

path::path(const std::string &s) : p(s)
{
}

std::string path::str() const
{
    return boost::filesystem::path(p).string();
}

std::wstring path::wstr() const
{
    return boost::filesystem::path(p).wstring();
}

path path::parent() const
{
    return boost::filesystem::path(p).parent_path().string();
}

std::string path::name() const
{
    return boost::filesystem::path(p).filename().string();
}

std::string path::stem() const
{
    return boost::filesystem::path(p).stem().string();
}

std::string path::extension() const
{
    return normalize_extension(boost::filesystem::path(p).extension().string());
}

bool path::operator ==(const path &other) const
{
    return boost::filesystem::path(p) == boost::filesystem::path(other.p);
}

bool path::operator <(const path &other) const
{
    // return boost::filesystem::path(p) < boost::filesystem::path(other.p);
    return p < other.p;
}

path path::operator /(const path &other) const
{
    const boost::filesystem::path p1(p);
    const boost::filesystem::path p2(other.p);
    return path((p1 / p2).string());
}

void path::operator /=(const path &other)
{
    p = (*this / other).p;
}

bool path::has_extension() const
{
    return !extension().empty();
}

bool path::has_extension(const std::string &target_extension) const
{
    return boost::iequals(extension(), normalize_extension(target_extension));
}

void path::change_extension(const std::string &new_extension)
{
    if (name().empty() || stem().empty() || name() == "." || name() == "..")
        return;

    const auto last_dot_pos = name().find_last_of('.');
    auto extension = normalize_extension(new_extension);
    p = (parent() / (name().substr(0, last_dot_pos) + extension)).str();
}
