#include <string>
#include "file.h"
#include "io/buffered_io.h"
#include "io/file_io.h"

using namespace au;

static void change_extension(
    boost::filesystem::path &path, const std::string &new_extension)
{
    if (path.filename().empty()
        || new_extension.empty()
        || path.filename() == "."
        || path.filename() == ".."
        || path.stem() == "")
    {
        return;
    }

    if (new_extension[0] == '.')
    {
        path.replace_extension(new_extension);
    }
    else
    {
        path.replace_extension("." + new_extension);
    }
}

File::File(const boost::filesystem::path &path, const io::FileMode mode)
    : io(*new io::FileIO(path, mode)), name(path.string())
{
}

File::File() : io(*new io::BufferedIO)
{
}

File::~File()
{
    delete &io;
}

bool File::has_extension(const std::string &extension)
{
    auto path = boost::filesystem::path(name);
    ::change_extension(path, extension);
    return name == path;
}

void File::change_extension(const std::string &new_extension)
{
    auto path = boost::filesystem::path(name);
    ::change_extension(path, new_extension);
    name = path.string();
}

void File::guess_extension()
{
    std::vector<std::pair<std::string, std::string>> definitions
    {
        { "b",      "abmp"_s         }, //QLiE
        { "imoavi", "IMOAVI"_s       }, //QLiE
        { "png",    "\x89PNG"_s      },
        { "bmp",    "BM"_s           },
        { "wav",    "RIFF"_s         },
        { "ogg",    "OggS"_s         },
        { "jpeg",   "\xff\xd8\xff"_s },
    };

    size_t old_pos = io.tell();
    for (auto &def : definitions)
    {
        const std::string ext = def.first;
        const std::string magic = def.second;
        io.seek(0);
        if (io.size() < magic.size()) continue;
        if (io.read(magic.size()) != magic) continue;
        change_extension(ext);
        io.seek(old_pos);
        return;
    }
    io.seek(old_pos);
}
