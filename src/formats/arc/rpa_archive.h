#ifndef FORMATS_ARC_RPA_ARCHIVE
#define FROMATS_ARC_RPA_ARCHIVE
#include "formats/archive.h"

class RpaArchive final : public Archive
{
public:
    bool unpack_internal(IO *, OutputFiles &output_files) override;
};

#endif
