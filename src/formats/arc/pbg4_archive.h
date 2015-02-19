#ifndef FORMATS_ARC_PBG4_ARCHIVE
#define FORMATS_ARC_PBG4_ARCHIVE
#include "formats/archive.h"

class Pbg4Archive final : public Archive
{
public:
    void unpack_internal(File &, FileSaver &) const override;
};

#endif
