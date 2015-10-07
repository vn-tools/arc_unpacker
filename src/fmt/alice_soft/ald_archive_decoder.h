#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class AldArchiveDecoder final : public ArchiveDecoder
    {
    public:
        AldArchiveDecoder();
        ~AldArchiveDecoder();
        std::unique_ptr<fmt::ArchiveMeta> read_meta(File &arc_file) const;
        std::unique_ptr<File> read_file(
            File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const;
    protected:
        bool is_recognized_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
