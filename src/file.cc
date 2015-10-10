#include "file.h"
#include <boost/algorithm/string.hpp>
#include <string>
#include "io/buffered_io.h"
#include "io/file_io.h"

using namespace au;

static const std::vector<std::pair<std::string, bstr>> magic_definitions
{
    { "b",      "abmp"_b         }, // QLiE
    { "imoavi", "IMOAVI"_b       }, // QLiE
    { "png",    "\x89PNG"_b      },
    { "bmp",    "BM"_b           },
    { "wav",    "RIFF"_b         },
    { "ogg",    "OggS"_b         },
    { "jpeg",   "\xFF\xD8\xFF"_b },
};

static void change_extension(
    boost::filesystem::path &path, const std::string &new_extension)
{
    if (path.filename().empty()
        || path.stem().empty()
        || path.filename() == "."
        || path.filename() == "..")
    {
        return;
    }

    const auto filename = path.filename().string();
    const auto last_dot_pos = filename.find_last_of('.');
    auto extension = new_extension;
    while (extension[0] == '.')
        extension.erase(0, 1);
    if (!extension.empty() && extension[0] != '.')
        extension = "." + extension;
    path = path.parent_path() / (filename.substr(0, last_dot_pos) + extension);
}

File::File(const boost::filesystem::path &path, const io::FileMode mode)
    : io(*new io::FileIO(path, mode)), name(path.string())
{
}

File::File(const std::string &name, const bstr &data)
    : io(*new io::BufferedIO(data)), name(name)
{
}

File::File() : io(*new io::BufferedIO)
{
}

File::~File()
{
    delete &io;
}

bool File::has_extension()
{
    return !boost::filesystem::path(name).extension().empty();
}

bool File::has_extension(const std::string &extension)
{
    auto path = boost::filesystem::path(name);
    ::change_extension(path, extension);
    return boost::iequals(name, path.string());
}

void File::change_extension(const std::string &new_extension)
{
    auto path = boost::filesystem::path(name);
    ::change_extension(path, new_extension);
    name = path.string();
}

void File::guess_extension()
{
    const size_t old_pos = io.tell();
    for (auto &def : magic_definitions)
    {
        const std::string ext = def.first;
        const bstr magic = def.second;
        io.seek(0);
        if (io.size() < magic.size()) continue;
        if (io.read(magic.size()) != magic) continue;
        change_extension(ext);
        io.seek(old_pos);
        return;
    }
    io.seek(old_pos);
}
