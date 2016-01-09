#pragma once

#include "dec/base_archive_decoder.h"
#include "dec/nitroplus/npa_plugin.h"
#include "plugin_manager.h"

namespace au {
namespace dec {
namespace nitroplus {

    class NpaArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        NpaArchiveDecoder();

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

    private:
        PluginManager<std::shared_ptr<NpaPlugin>> plugin_manager;
    };

} } }
