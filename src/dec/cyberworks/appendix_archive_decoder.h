#pragma once

#include "dec/base_archive_decoder.h"
#include "dec/cyberworks/dat_plugin.h"
#include "plugin_manager.h"

namespace au {
namespace dec {
namespace cyberworks {

    class AppendixArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        AppendixArchiveDecoder();

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;

    public:
        PluginManager<DatPlugin> plugin_manager;
    };

} } }
