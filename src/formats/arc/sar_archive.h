#ifndef FORMATS_ARC_SAR_ARCHIVE
#define FORMATS_ARC_SAR_ARCHIVE
#include "formats/archive.h"

class SarArchive final : public Archive
{
public:
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
};

#endif
