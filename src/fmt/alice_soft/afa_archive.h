#ifndef AU_FMT_ALICE_SOFT_AFA_ARCHIVE_H
#define AU_FMT_ALICE_SOFT_AFA_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class AfaArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
