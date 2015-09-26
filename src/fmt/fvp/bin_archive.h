#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace fvp {

    class BinArchive final : public ArchiveDecoder
    {
    public:
        BinArchive();
        ~BinArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
