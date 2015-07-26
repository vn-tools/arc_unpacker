#ifndef AU_FMT_TOUHOU_ANM_ARCHIVE_H
#define AU_FMT_TOUHOU_ANM_ARCHIVE_H
#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace touhou {

    class AnmArchive final : public Archive
    {
    protected:
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
        FileNamingStrategy get_file_naming_strategy() const override;
    };

} } }

#endif
