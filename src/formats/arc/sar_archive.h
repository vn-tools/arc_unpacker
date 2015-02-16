#ifndef FORMATS_ARC_SAR_ARCHIVE
#define FORMATS_ARC_SAR_ARCHIVE
#include "formats/archive.h"

class SarArchive final : public Archive
{
public:
    void unpack_internal(File &, FileSaver &) const override;
};

#endif
