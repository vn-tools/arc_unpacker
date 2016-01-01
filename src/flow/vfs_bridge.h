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
