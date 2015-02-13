#ifndef FORMATS_ARC_PAK_ARCHIVE
#define FORMATS_ARC_PAK_ARCHIVE
#include "formats/archive.h"

class PakArchive final : public Archive
{
public:
    void unpack_internal(VirtualFile &, OutputFiles &) const override;
};

#endif
