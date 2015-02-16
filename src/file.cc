#include <string>
#include "buffered_io.h"
#include "file.h"
#include "file_io.h"
#include "fs.h"

File::File(const std::string path, const std::string mode)
    : io(*new FileIO(path, mode)), name(path)
{
}

File::File() : io(*new BufferedIO)
{
}

File::~File()
{
    delete &io;
}

void File::change_extension(const std::string new_extension)
{
    std::string ext_copy(new_extension);
    while (ext_copy.length() > 0 && ext_copy[0] == '.')
        ext_copy.erase(0, 1);

    std::string base_name(basename(name));

    bool all_dots = true;
    for (auto &c : base_name)
        all_dots &= c == '.';
    if (all_dots)
    {
        return;
    }

    std::string new_name(dirname(name));
    if (new_name.length() > 0)
    {
        if (new_name.back() != '/' && new_name.back() != '\\')
            new_name += "/";
    }

    size_t pos = base_name.rfind(".");
    if (pos == std::string::npos)
        new_name += base_name + "." + ext_copy;
    else
        new_name += base_name.substr(0, pos) + "." + ext_copy;

    name = new_name;
}

void File::guess_extension()
{
    std::vector<std::pair<std::string, std::string>> definitions
    {
        { "b", std::string("abmp", 4) }, //QLiE
        { "imoavi", std::string("IMOAVI", 6) }, //QLiE
        { "png", std::string("\x89PNG", 4) },
        { "bmp", std::string("BM", 2) },
        { "wav", std::string("RIFF", 4) },
        { "ogg", std::string("OggS", 4) },
        { "jpeg", std::string("\xff\xd8\xff", 3) },
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
