#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace majiro {

    class ArcArchiveDecoder final : public ArchiveDecoder
    {
    public:
        ArcArchiveDecoder();
        ~ArcArchiveDecoder();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
