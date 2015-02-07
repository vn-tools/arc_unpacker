#ifndef FORMATS_ARC_XP3_ARCHIVE
#define FROMATS_ARC_XP3_ARCHIVE
#include "formats/archive.h"

class Xp3Archive final : public Archive
{
public:
    void unpack_internal(IO &arc_io, OutputFiles &output_files) const override;
};

#endif
