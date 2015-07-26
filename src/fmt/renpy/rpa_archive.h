#ifndef AU_FMT_RENPY_RPA_ARCHIVE_H
#define AU_FMT_RENPY_RPA_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace renpy {

    class RpaArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
