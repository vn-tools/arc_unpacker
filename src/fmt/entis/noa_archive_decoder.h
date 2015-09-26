#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace entis {

    class NoaArchiveDecoder final : public ArchiveDecoder
    {
    public:
        NoaArchiveDecoder();
        ~NoaArchiveDecoder();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
