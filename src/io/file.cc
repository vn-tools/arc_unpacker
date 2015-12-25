#include "io/file.h"
#include <string>
#include "io/file_stream.h"
#include "io/memory_stream.h"

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

File::File(File &other_file) :
    stream_holder(other_file.stream.clone()),
    stream(*stream_holder),
    path(other_file.path)
{
}

File::File(const io::path &path, const io::FileMode mode) :
    stream_holder(new io::FileStream(path, mode)),
    stream(*stream_holder),
    path(path)
{
}

File::File(const io::path &path, const bstr &data) :
    stream_holder(new io::MemoryStream(data)),
    stream(*stream_holder),
    path(path)
{
}

File::File() : stream_holder(new io::MemoryStream()), stream(*stream_holder)
{
}

File::~File()
{
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
