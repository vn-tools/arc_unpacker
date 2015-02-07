#ifndef FORMATS_ARC_PAK_ARCHIVE
#define FROMATS_ARC_PAK_ARCHIVE
#include "formats/archive.h"

class PakArchive final : public Archive
{
public:
    bool unpack_internal(IO &arc_io, OutputFiles &output_files) override;
};

#endif
