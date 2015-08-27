#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace riddle_soft {

    class PacArchive final : public Archive
    {
    public:
        PacArchive();
        ~PacArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
