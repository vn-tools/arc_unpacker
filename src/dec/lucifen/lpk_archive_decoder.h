#pragma once

#include "dec/base_archive_decoder.h"
#include "dec/lucifen/lpk_plugin.h"
#include "plugin_manager.h"

namespace au {
namespace dec {
namespace lucifen {

    class LpkArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        LpkArchiveDecoder();

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
        PluginManager<LpkPlugin> plugin_manager;
    };

} } }
