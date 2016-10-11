// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "io/file.h"
#include <string>
#include "io/file_byte_stream.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::io;

static const std::vector<std::pair<bstr, std::string>> magic_definitions
{
    {"abmp"_b,                       "b"},      // QLiE
    {"IMOAVI"_b,                     "imoavi"}, // QLiE
    {"\x89PNG"_b,                    "png"},
    {"BM"_b,                         "bmp"},
    {"RIFF"_b,                       "wav"},
    {"OggS"_b,                       "ogg"},
    {"\xFF\xD8\xFF"_b,               "jpeg"},
    {"\x00\x00\x00\x1C""ftypmp42"_b, "mp4"},
    {"\x00\x00\x00\x14""ftypisom"_b, "mp4"},
};

File::File(File &other_file) :
    stream_holder(other_file.stream.clone()),
    stream(*stream_holder),
    path(other_file.path)
{
}

File::File(const io::path &path, std::unique_ptr<BaseByteStream> stream) :
    stream_holder(std::move(stream)),
    stream(*stream_holder),
    path(path)
{
}

File::File(const io::path &path, const FileMode mode) :
    File(path, std::make_unique<FileByteStream>(path, mode))
{
}

File::File(const io::path &path, const bstr &data) :
    File(path, std::make_unique<MemoryByteStream>(data))
{
}

File::File() : File("", std::make_unique<MemoryByteStream>())
{
}

File::~File()
{
}

void File::guess_extension()
{
    const auto old_pos = stream.pos();
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
