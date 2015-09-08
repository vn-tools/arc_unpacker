#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace majiro {

    class ArcArchive final : public Archive
    {
    public:
        ArcArchive();
        ~ArcArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
