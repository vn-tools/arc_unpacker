#ifndef FORMATS_TANUKISOFT_XFL_ARCHIVE_H
#define FORMATS_TANUKISOFT_XFL_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace TanukiSoft
    {
        class TacArchive final : public Archive
        {
        public:
            TacArchive();
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
