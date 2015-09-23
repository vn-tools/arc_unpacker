#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace fc01 {

    class MrgArchive final : public Archive
    {
    public:
        MrgArchive();
        ~MrgArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
