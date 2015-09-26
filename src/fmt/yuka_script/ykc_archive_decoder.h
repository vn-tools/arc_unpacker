#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace yuka_script {

    class YkcArchiveDecoder final : public ArchiveDecoder
    {
    public:
        YkcArchiveDecoder();
        ~YkcArchiveDecoder();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
