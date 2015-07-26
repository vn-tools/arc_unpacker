#ifndef AU_FMT_TANUKISOFT_XFL_ARCHIVE_H
#define AU_FMT_TANUKISOFT_XFL_ARCHIVE_H
#include "formats/archive.h"

namespace au {
namespace fmt {
namespace tanuki_soft {

    class TacArchive final : public Archive
    {
    public:
        TacArchive();
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    };

} } }

#endif
