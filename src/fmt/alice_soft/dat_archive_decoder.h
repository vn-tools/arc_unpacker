#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class DatArchiveDecoder final : public ArchiveDecoder
    {
    public:
        DatArchiveDecoder();
        ~DatArchiveDecoder();
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
