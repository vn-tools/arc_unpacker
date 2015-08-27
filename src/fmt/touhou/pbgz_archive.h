#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace touhou {

    class PbgzArchive final : public Archive
    {
    public:
        PbgzArchive();
        ~PbgzArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
