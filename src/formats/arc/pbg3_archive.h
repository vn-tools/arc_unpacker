#ifndef FORMATS_ARC_PBG3_ARCHIVE
#define FORMATS_ARC_PBG3_ARCHIVE
#include "formats/archive.h"

class Pbg3Archive final : public Archive
{
public:
    void unpack_internal(File &, FileSaver &) const override;
};

#endif
