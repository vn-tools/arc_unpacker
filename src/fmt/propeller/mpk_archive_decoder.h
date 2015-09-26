#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace propeller {

    class MpkArchiveDecoder final : public ArchiveDecoder
    {
    public:
        MpkArchiveDecoder();
        ~MpkArchiveDecoder();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
