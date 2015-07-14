#ifndef FORMATS_PROPELLER_MPK_ARCHIVE_H
#define FORMATS_PROPELLER_MPK_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Propeller
    {
        class MpkArchive final : public Archive
        {
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
