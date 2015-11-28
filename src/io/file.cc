#include "file.h"
#include <string>
#include "io/memory_stream.h"
#include "io/file_stream.h"

using namespace au;
using namespace au::io;

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

File::File(const io::path &name, const io::FileMode mode)
    : stream(*new io::FileStream(name, mode)), name(name)
{
}

File::File(const io::path &name, const bstr &data)
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
    return name.has_extension();
}

bool File::has_extension(const std::string &extension)
{
    return name.has_extension(extension);
}

void File::change_extension(const std::string &new_extension)
{
    auto path = name;
    path.change_extension(new_extension);
    name = path.str();
}

void File::guess_extension()
{
    const size_t old_pos = stream.tell();
    for (auto &def : magic_definitions)
    {
        const std::string ext = def.first;
        const bstr magic = def.second;
        stream.seek(0);
        if (stream.size() < magic.size())
            continue;
        if (stream.read(magic.size()) != magic)
            continue;
        change_extension(ext);
        stream.seek(old_pos);
        return;
    }
    stream.seek(old_pos);
}
