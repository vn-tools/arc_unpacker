#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace liar_soft {

    class XflArchiveDecoder final : public ArchiveDecoder
    {
    public:
        XflArchiveDecoder();
        ~XflArchiveDecoder();
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
