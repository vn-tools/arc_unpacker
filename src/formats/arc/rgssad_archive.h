#ifndef FORMATS_ARC_RGSSAD_ARCHIVE
#define FORMATS_ARC_RGSSAD_ARCHIVE
#include "formats/archive.h"

class RgssadArchive final : public Archive
{
public:
    void unpack_internal(File &, FileSaver &) const override;
};

#endif
