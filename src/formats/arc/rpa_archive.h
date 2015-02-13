#ifndef FORMATS_ARC_RPA_ARCHIVE
#define FORMATS_ARC_RPA_ARCHIVE
#include "formats/archive.h"

class RpaArchive final : public Archive
{
public:
    void unpack_internal(VirtualFile &, OutputFiles &) const override;
};

#endif
