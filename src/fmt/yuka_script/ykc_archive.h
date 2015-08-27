#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace yuka_script {

    class YkcArchive final : public Archive
    {
    public:
        YkcArchive();
        ~YkcArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
