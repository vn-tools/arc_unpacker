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

#pragma once

#include "dec/base_archive_decoder.h"
#include "logger.h"
#include "virtual_file_system.h"

namespace au {
namespace flow {

    // A RAII based VirtualFileSystem registerer that cleans up after itself
    // when the files are no longer needed.
    class VirtualFileSystemBridge final
    {
    public:
        VirtualFileSystemBridge(
            const Logger &logger,
            const dec::BaseArchiveDecoder &decoder,
            const std::shared_ptr<dec::ArchiveMeta> meta,
            const std::shared_ptr<io::File> input_file,
            const io::path &base_name);

        ~VirtualFileSystemBridge();

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
