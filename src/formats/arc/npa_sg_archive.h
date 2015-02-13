#ifndef FORMATS_ARC_NPA_SG_ARCHIVE
#define FORMATS_ARC_NPA_SG_ARCHIVE
#include "formats/archive.h"

class NpaSgArchive final : public Archive
{
public:
    void unpack_internal(VirtualFile &, OutputFiles &) const override;
};

#endif
