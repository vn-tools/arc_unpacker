#ifndef AU_FMT_TOUHOU_PAK1_IMAGE_ARCHIVE_H
#define AU_FMT_TOUHOU_PAK1_IMAGE_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace touhou {

    class Pak1ImageArchive : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
