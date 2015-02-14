#ifndef FORMATS_ARC_PAC_ARCHIVE
#define FORMATS_ARC_PAC_ARCHIVE
#include "formats/archive.h"

class PacArchive final : public Archive
{
public:
    void unpack_internal(VirtualFile &, OutputFiles &) const override;
};

#endif
