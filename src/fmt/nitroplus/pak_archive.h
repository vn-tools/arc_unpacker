#ifndef AU_FMT_NITROPLUS_PAK_ARCHIVE_H
#define AU_FMT_NITROPLUS_PAK_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace nitroplus {

    class PakArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
