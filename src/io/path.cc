// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "io/path.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>

using namespace au;
using namespace au::io;

static std::string fix_slashes(const std::string &input)
{
    return boost::algorithm::replace_all_copy(input, "\\", "/");
}

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

path::path(const char *s) : p(fix_slashes(s))
{
}

path::path(const std::string &s) : p(fix_slashes(s))
{
}

const char *path::c_str() const
{
    return p.c_str();
}

std::string path::str() const
{
    return boost::filesystem::path(p).string();
}

std::wstring path::wstr() const
{
    return boost::filesystem::path(p).wstring();
}

path path::make_relative(const path &other_path) const
{
    boost::filesystem::path source = p;
    boost::filesystem::path target = other_path.p;
    boost::filesystem::path::const_iterator source_it = source.begin();
    boost::filesystem::path::const_iterator target_it = target.begin();

    while (source_it != source.end()
        && target_it != target.end()
        && *target_it == *source_it)
    {
        ++target_it;
        ++source_it;
    }

    boost::filesystem::path final_path;
    while (source_it != source.end())
    {
        final_path /= "..";
        ++source_it;
    }

    while (target_it != target.end())
    {
        final_path /= *target_it;
        ++target_it;
    }

    return final_path.string();
}

path path::parent() const
{
    return boost::filesystem::path(p).parent_path().string();
}

std::string path::name() const
{
    const auto ret = boost::filesystem::path(p).filename().string();
    if (ret == "." || ret == ".." || ret == "/")
        return "";
    return ret;
}

std::string path::stem() const
{
    const auto ret = boost::filesystem::path(p).stem().string();
    if (ret == "." || ret == ".." || ret == "/")
        return "";
    return ret;
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
    return boost::filesystem::path(p) < boost::filesystem::path(other.p);
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

bool path::is_absolute() const
{
    return boost::filesystem::path(p).is_absolute();
}

bool path::is_root() const
{
    return boost::filesystem::path(p) == boost::filesystem::path(p).root_path();
}

bool path::has_extension() const
{
    return !extension().empty();
}

bool path::has_extension(const std::string &target_extension) const
{
    return boost::iequals(extension(), normalize_extension(target_extension));
}

path &path::change_stem(const std::string &new_stem)
{
    p = (parent() / (new_stem + extension())).str();
    return *this;
}

path &path::change_extension(const std::string &new_extension)
{
    if (name().empty() || stem().empty())
        return *this;

    const auto last_dot_pos = name().find_last_of('.');
    const auto extension = normalize_extension(new_extension);
    p = (parent() / (name().substr(0, last_dot_pos) + extension)).str();
    return *this;
}
