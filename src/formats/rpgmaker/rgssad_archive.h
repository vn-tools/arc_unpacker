#ifndef AU_FMT_RPGMAKER_RGSSAD_ARCHIVE_H
#define AU_FMT_RPGMAKER_RGSSAD_ARCHIVE_H
#include "formats/archive.h"

namespace au {
namespace fmt {
namespace rpgmaker {

    class RgssadArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
