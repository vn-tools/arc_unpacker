#ifndef FORMATS_ARC_PACK_ARCHIVE
#define FORMATS_ARC_PACK_ARCHIVE
#include "formats/archive.h"

class PackArchive final : public Archive
{
public:
    void unpack_internal(VirtualFile &, OutputFiles &) const override;
};

#endif
