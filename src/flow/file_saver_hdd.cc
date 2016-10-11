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

#include "flow/file_saver_hdd.h"
#include <mutex>
#include <set>
#include "algo/format.h"
#include "io/file_byte_stream.h"
#include "io/file_system.h"

using namespace au;
using namespace au::flow;

static std::mutex mutex;

struct FileSaverHdd::Priv final
{
    Priv(
        const io::path &output_dir,
        const bool overwrite);

    io::path make_path_unique(const io::path &path);

    io::path output_dir;
    bool overwrite;
    size_t saved_file_count;
    std::set<io::path> paths;
};

FileSaverHdd::Priv::Priv(const io::path &output_dir, const bool overwrite)
    : output_dir(output_dir), overwrite(overwrite), saved_file_count(0)
{
}

io::path FileSaverHdd::Priv::make_path_unique(const io::path &path)
{
    io::path new_path = path;
    int i = 1;
    while (paths.find(new_path) != paths.end()
        || (!overwrite && io::exists(new_path)))
    {
        new_path.change_stem(path.stem() + algo::format("(%d)", i++));
    }
    paths.insert(new_path);
    return new_path;
}

FileSaverHdd::FileSaverHdd(
    const io::path &output_dir, const bool overwrite)
    : p(new Priv(output_dir, overwrite))
{
}

FileSaverHdd::~FileSaverHdd()
{
}

io::path FileSaverHdd::save(std::shared_ptr<io::File> file) const
{
    std::unique_lock<std::mutex> lock(mutex);
    const auto full_path = p->make_path_unique(p->output_dir / file->path);
    io::create_directories(full_path.parent());
    io::FileByteStream output_stream(full_path, io::FileMode::Write);
    file->stream.seek(0);
    output_stream.write(file->stream);
    ++p->saved_file_count;
    return full_path;
}

size_t FileSaverHdd::get_saved_file_count() const
{
    return p->saved_file_count;
}
