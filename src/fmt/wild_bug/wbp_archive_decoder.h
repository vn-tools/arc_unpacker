#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace wild_bug {

    class WbpArchiveDecoder final : public ArchiveDecoder
    {
    public:
        WbpArchiveDecoder();
        ~WbpArchiveDecoder();
        std::unique_ptr<ArchiveMeta> read_meta(File &) const override;
        std::unique_ptr<File> read_file(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    protected:
        bool is_recognized_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
