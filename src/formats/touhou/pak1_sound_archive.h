#ifndef FORMATS_TOUHOU_PAK1_SOUND_ARCHIVE_H
#define FORMATS_TOUHOU_PAK1_SOUND_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class Pak1SoundArchive : public Archive
        {
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
