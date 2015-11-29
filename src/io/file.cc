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

File::File(const io::path &path, const io::FileMode mode)
    : stream(*new io::FileStream(path, mode)), path(path)
{
}

File::File(const io::path &path, const bstr &data)
    : stream(*new io::MemoryStream(data)), path(path)
{
}

File::File() : stream(*new io::MemoryStream)
{
}

File::~File()
{
    delete &stream;
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
        path.change_extension(ext);
        stream.seek(old_pos);
        return;
    }
    stream.seek(old_pos);
}
