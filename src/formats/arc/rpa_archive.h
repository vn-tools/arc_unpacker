#ifndef FORMATS_ARC_RPA_ARCHIVE
#define FORMATS_ARC_RPA_ARCHIVE
#include "formats/archive.h"

class RpaArchive final : public Archive
{
public:
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
};

#endif
