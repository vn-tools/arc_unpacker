#include "util/virtual_file_system.h"
#include <map>
#include <set>
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
        factories[path] = factory;
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

    for (const auto &kv : factories)
        if (kv.first.stem() == stem)
            return kv.second();

    for (const auto &directory : directories)
    for (const auto &other_path : io::recursive_directory_range(directory))
        if (other_path.stem() == stem)
            return std::make_unique<io::File>(other_path, io::FileMode::Read);

    return nullptr;
}

std::unique_ptr<io::File> VirtualFileSystem::get_by_name(
    const std::string &name)
{
    if (!enabled)
        return nullptr;

    for (const auto &kv : factories)
        if (kv.first.name() == name)
            return kv.second();

    for (const auto &directory : directories)
    for (const auto &other_path : io::recursive_directory_range(directory))
        if (other_path.name() == name)
            return std::make_unique<io::File>(other_path, io::FileMode::Read);

    return nullptr;
}

std::unique_ptr<io::File> VirtualFileSystem::get_by_path(const io::path &path)
{
    if (!enabled)
        return nullptr;

    if (factories.find(path) != factories.end())
        return factories[path]();

    for (const auto &directory : directories)
        for (const auto &other_path : io::recursive_directory_range(directory))
            if (other_path == path)
                return std::make_unique<io::File>(path, io::FileMode::Read);

    return nullptr;
}
