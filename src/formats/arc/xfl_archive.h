#ifndef FORMATS_ARC_XFL_ARCHIVE
#define FORMATS_ARC_XFL_ARCHIVE
#include "formats/archive.h"

class XflArchive final : public Archive
{
public:
    void unpack_internal(File &, FileSaver &) const override;
};

#endif
