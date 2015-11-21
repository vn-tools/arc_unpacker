#include "file.h"
#include <boost/algorithm/string.hpp>
#include <string>
#include "io/memory_stream.h"
#include "io/file_stream.h"

using namespace au;

static const std::vector<std::pair<std::string, bstr>> magic_definitions
{
    {"b",      "abmp"_b},   // QLiE
    {"imoavi", "IMOAVI"_b}, // QLiE
    {"png",    "\x89PNG"_b},
    {"bmp",    "BM"_b},
    {"wav",    "RIFF"_b},
    {"ogg",    "OggS"_b},
    {"jpeg",   "\xFF\xD8\xFF"_b},
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
    : stream(*new io::FileStream(path, mode)), name(path.string())
{
}

File::File(const std::string &name, const bstr &data)
    : stream(*new io::MemoryStream(data)), name(name)
{
}

File::File() : stream(*new io::MemoryStream)
{
}

File::~File()
{
    delete &stream;
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
    const size_t old_pos = stream.tell();
    for (auto &def : magic_definitions)
    {
        const std::string ext = def.first;
        const bstr magic = def.second;
        stream.seek(0);
        if (stream.size() < magic.size()) continue;
        if (stream.read(magic.size()) != magic) continue;
        change_extension(ext);
        stream.seek(old_pos);
        return;
    }
    stream.seek(old_pos);
}
