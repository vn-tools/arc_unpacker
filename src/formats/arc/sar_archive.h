#ifndef FORMATS_ARC_SAR_ARCHIVE
#define FROMATS_ARC_SAR_ARCHIVE
#include "formats/archive.h"

class SarArchive final : public Archive
{
public:
    bool unpack_internal(IO *, OutputFiles &output_files) override;
};

#endif
