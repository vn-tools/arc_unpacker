#ifndef AU_FMT_MICROSOFT_EXE_ARCHIVE_H
#define AU_FMT_MICROSOFT_EXE_ARCHIVE_H
#include "formats/archive.h"

namespace au {
namespace fmt {
namespace microsoft {

    class ExeArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
