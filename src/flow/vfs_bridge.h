#pragma once

#include "fmt/base_archive_decoder.h"
#include "logger.h"
#include "util/virtual_file_system.h"

namespace au {
namespace flow {

    // A RAII based VirtualFileSystem registerer that cleans up after itself
    // when the files are no longer needed.
    class VirtualFileSystemBridge final
    {
    public:
        VirtualFileSystemBridge(
            const Logger &logger,
            const fmt::BaseArchiveDecoder &decoder,
            const std::shared_ptr<fmt::ArchiveMeta> meta,
            const std::shared_ptr<io::File> input_file,
            const io::path &base_name);

        ~VirtualFileSystemBridge();

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
