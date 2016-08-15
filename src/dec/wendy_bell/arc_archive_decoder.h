#pragma once

#include <array>
#include "dec/base_archive_decoder.h"
#include "plugin_manager.h"

namespace au {
namespace dec {
namespace wendy_bell {

    enum class ImageParameter : int
    {
        Width,
        Height,
        BitmapSize,
        AlphaOffset,
        Type,
        Unknown,
    };

    struct ArcArchivePlugin final
    {
        // parameters are in different order for each game
        std::vector<ImageParameter> img_param_order;
        u8 img_delim[3];
    };

    class ArcArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        ArcArchiveDecoder();

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
        PluginManager<ArcArchivePlugin> plugin_manager;
    };

} } }
