#ifndef AU_FMT_ALICE_SOFT_ALD_ARCHIVE_H
#define AU_FMT_ALICE_SOFT_ALD_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class AldArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
