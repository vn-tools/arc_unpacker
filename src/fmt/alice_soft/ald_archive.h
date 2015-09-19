#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class AldArchive final : public Archive
    {
    public:
        AldArchive();
        ~AldArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
