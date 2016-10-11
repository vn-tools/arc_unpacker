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

#include "flow/file_saver_callback.h"

using namespace au;
using namespace au::flow;

struct FileSaverCallback::Priv final
{
    Priv(FileSaveCallback callback);

    FileSaveCallback callback;
    size_t saved_file_count;
};

FileSaverCallback::Priv::Priv(FileSaveCallback callback)
    : callback(callback), saved_file_count(0)
{
}

FileSaverCallback::FileSaverCallback() : p(new Priv(nullptr))
{
}

FileSaverCallback::FileSaverCallback(FileSaveCallback callback)
    : p(new Priv(callback))
{
}

FileSaverCallback::~FileSaverCallback()
{
}

void FileSaverCallback::set_callback(FileSaveCallback callback)
{
    p->callback = callback;
}

io::path FileSaverCallback::save(std::shared_ptr<io::File> file) const
{
    p->callback(file);
    ++p->saved_file_count;
    return file->path;
}

size_t FileSaverCallback::get_saved_file_count() const
{
    return p->saved_file_count;
}
