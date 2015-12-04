#include "file.h"
#include <string>
#include "io/memory_stream.h"
#include "io/file_stream.h"

using namespace au;
using namespace au::io;

static const std::vector<std::pair<bstr, std::string>> magic_definitions
{
    {"abmp"_b,         "b"},      // QLiE
    {"IMOAVI"_b,       "imoavi"}, // QLiE
    {"\x89PNG"_b,      "png"},
    {"BM"_b,           "bmp"},
    {"RIFF"_b,         "wav"},
    {"OggS"_b,         "ogg"},
    {"\xFF\xD8\xFF"_b, "jpeg"},
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
    const auto old_pos = stream.tell();
    for (const auto &def : magic_definitions)
    {
        const auto magic = def.first;
        const auto ext = def.second;
        if (stream.size() < magic.size())
            continue;
        if (stream.seek(0).read(magic.size()) != magic)
            continue;
        path.change_extension(ext);
        stream.seek(old_pos);
        return;
    }
    stream.seek(old_pos);
}
