#include "util/virtual_file_system.h"
#include <map>
#include <set>
#include "algo/str.h"
#include "err.h"
#include "io/file_system.h"

using namespace au;
using namespace au::util;

static std::map<io::path, std::function<std::unique_ptr<io::File>()>> factories;
static std::set<io::path> directories;
static bool enabled = true;

void VirtualFileSystem::disable()
{
    enabled = false;
}

void VirtualFileSystem::enable()
{
    enabled = true;
}

void VirtualFileSystem::register_file(
    const io::path &path,
    const std::function<std::unique_ptr<io::File>()> factory)
{
    if (enabled)
        factories[io::path(algo::lower(path.str()))] = factory;
}

void VirtualFileSystem::unregister_file(const io::path &path)
{
    factories.erase(path);
}

void VirtualFileSystem::register_directory(const io::path &path)
{
    if (enabled)
        directories.insert(path);
}

void VirtualFileSystem::unregister_directory(const io::path &path)
{
    directories.erase(path);
}

std::unique_ptr<io::File> VirtualFileSystem::get_by_stem(
    const std::string &stem)
{
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
    if (!enabled)
        return nullptr;

    const auto check = io::path(algo::lower(path.str()));
    if (factories.find(check) != factories.end())
        return factories[check]();

    for (const auto &directory : directories)
    for (const auto &other_path : io::recursive_directory_range(directory))
        if (io::path(algo::lower(other_path.str())) == check)
            return std::make_unique<io::File>(other_path, io::FileMode::Read);

    return nullptr;
}
