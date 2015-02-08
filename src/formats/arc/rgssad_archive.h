#ifndef FORMATS_ARC_RGSSAD_ARCHIVE
#define FROMATS_ARC_RGSSAD_ARCHIVE
#include "formats/archive.h"

class RgssadArchive final : public Archive
{
public:
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
};

#endif
