#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class AlkArchive final : public Archive
    {
    public:
        AlkArchive();
        ~AlkArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
