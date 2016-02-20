#pragma once

#include "dec/base_archive_decoder.h"
#include "plugin_manager.h"

namespace au {
namespace dec {
namespace nitroplus {

    class Npk2ArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        Npk2ArchiveDecoder();
        std::vector<std::string> get_linked_formats() const override;

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
        PluginManager<bstr> plugin_manager;
    };

} } }
