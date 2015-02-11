#ifndef FORMATS_ARC_P_ARCHIVE
#define FORMATS_ARC_P_ARCHIVE
#include "formats/archive.h"

class PArchive final : public Archive
{
public:
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
};

#endif
