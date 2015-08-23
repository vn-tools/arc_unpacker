#ifndef AU_FMT_LIBIDO_ARC_ARCHIVE_H
#define AU_FMT_LIBIDO_ARC_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace libido {

    class ArcArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
