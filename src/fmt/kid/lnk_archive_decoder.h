#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace kid {

    class LnkArchiveDecoder final : public ArchiveDecoder
    {
    public:
        LnkArchiveDecoder();
        ~LnkArchiveDecoder();
    protected:
        bool is_recognized_impl(File &) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
