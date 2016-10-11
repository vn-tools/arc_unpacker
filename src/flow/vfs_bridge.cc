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

#include "flow/vfs_bridge.h"

using namespace au;
using namespace au::flow;

struct VirtualFileSystemBridge::Priv final
{
    Priv(
        const Logger &logger,
        const dec::BaseArchiveDecoder &decoder,
        const std::shared_ptr<dec::ArchiveMeta> meta,
        const std::shared_ptr<io::File> input_file,
        const io::path &base_name) :
            logger(logger),
            decoder(decoder),
            meta(meta),
            base_name(base_name),
            decoder_refcount(decoder.shared_from_this())
    {
        for (const auto &entry : meta->entries)
        {
            VirtualFileSystem::register_file(
                get_target_name(entry->path),
                [&logger, input_file, meta, &entry, &decoder]()
                {
                    io::File file_copy(*input_file);
                    return decoder.read_file(logger, file_copy, *meta, *entry);
                });
        }
    }

    ~Priv()
    {
        for (const auto &entry : meta->entries)
        {
            VirtualFileSystem::unregister_file(
                get_target_name(entry->path));
        }
    }

    io::path get_target_name(const io::path &input_path) const
    {
        return algo::apply_naming_strategy(
            decoder.naming_strategy(), base_name, input_path);
    }

    const Logger logger;
    const dec::BaseArchiveDecoder &decoder;
    const std::shared_ptr<dec::ArchiveMeta> meta;
    const io::path base_name;

    // Prolongs decoder life to match those of the bridge. Avoids
    // the need to downcast to shared_ptr<BaseArchiveDecoder>.
    // TODO: probably not needed after we get long-lived task chains
    const std::shared_ptr<const dec::IDecoder> decoder_refcount;
};

VirtualFileSystemBridge::VirtualFileSystemBridge(
    const Logger &logger,
    const dec::BaseArchiveDecoder &decoder,
    const std::shared_ptr<dec::ArchiveMeta> meta,
    const std::shared_ptr<io::File> input_file,
    const io::path &base_name)
    : p(new Priv(logger, decoder, meta, input_file, base_name))
{
}

VirtualFileSystemBridge::~VirtualFileSystemBridge()
{
}
