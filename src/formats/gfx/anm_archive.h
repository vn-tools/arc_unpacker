#ifndef FORMATS_GFX_ANM_ARCHIVE
#define FORMATS_GFX_ANM_ARCHIVE
#include "formats/archive.h"

class AnmArchive final : public Archive
{
public:
    void unpack_internal(File &, FileSaver &) const override;
};

#endif
