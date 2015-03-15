#ifndef FORMATS_YUKASCRIPT_YKC_ARCHIVE_H
#define FORMATS_YUKASCRIPT_YKC_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace YukaScript
    {
        class YkcArchive final : public Archive
        {
        public:
            YkcArchive();
            ~YkcArchive();
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
