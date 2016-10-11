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

#include "virtual_file_system.h"
#include <map>
#include <mutex>
#include <set>
#include "algo/str.h"
#include "err.h"
#include "io/file_system.h"

using namespace au;

static std::mutex mutex;
static std::map<io::path, std::function<std::unique_ptr<io::File>()>> factories;
static std::set<io::path> directories;
static bool enabled = true;

void VirtualFileSystem::disable()
{
    std::unique_lock<std::mutex> lock(mutex);
    enabled = false;
}

void VirtualFileSystem::enable()
{
    std::unique_lock<std::mutex> lock(mutex);
    enabled = true;
}

void VirtualFileSystem::clear()
{
    directories.clear();
    factories.clear();
}

void VirtualFileSystem::register_file(
    const io::path &path,
    const std::function<std::unique_ptr<io::File>()> factory)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (enabled)
        factories[io::path(algo::lower(path.str()))] = factory;
}

void VirtualFileSystem::unregister_file(const io::path &path)
{
    std::unique_lock<std::mutex> lock(mutex);
    factories.erase(io::path(algo::lower(path.str())));
}

void VirtualFileSystem::register_directory(const io::path &path)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (enabled)
        directories.insert(path);
}

void VirtualFileSystem::unregister_directory(const io::path &path)
{
    std::unique_lock<std::mutex> lock(mutex);
    directories.erase(path);
}

std::unique_ptr<io::File> VirtualFileSystem::get_by_stem(
    const std::string &stem)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (!enabled)
        return nullptr;

    const auto check = algo::lower(stem);
    for (const auto &kv : factories)
        if (kv.first.stem() == check)
            return kv.second();

    for (const auto &directory : directories)
    for (const auto &other_path : io::recursive_directory_range(directory))
        if (algo::lower(other_path.stem()) == check)
            return std::make_unique<io::File>(other_path, io::FileMode::Read);

    return nullptr;
}

std::unique_ptr<io::File> VirtualFileSystem::get_by_name(
    const std::string &name)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (!enabled)
        return nullptr;

    const auto check = algo::lower(name);
    for (const auto &kv : factories)
        if (kv.first.name() == check)
            return kv.second();

    for (const auto &directory : directories)
    for (const auto &other_path : io::recursive_directory_range(directory))
        if (algo::lower(other_path.name()) == check)
            return std::make_unique<io::File>(other_path, io::FileMode::Read);

    return nullptr;
}

std::unique_ptr<io::File> VirtualFileSystem::get_by_path(const io::path &path)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (!enabled)
        return nullptr;

    const auto check = io::path(algo::lower(path.str()));
    if (factories.find(check) != factories.end())
        return factories[check]();

    for (const auto &directory : directories)
    for (const auto &other_path : io::recursive_directory_range(directory))
    {
        if (io::path(algo::lower(other_path.str())) == check)
            return std::make_unique<io::File>(other_path, io::FileMode::Read);
    }

    return nullptr;
}
