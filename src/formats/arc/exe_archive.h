#ifndef FORMATS_ARC_EXE_ARCHIVE
#define FORMATS_ARC_EXE_ARCHIVE
#include "formats/archive.h"

class ExeArchive final : public Archive
{
public:
    void unpack_internal(File &, FileSaver &) const override;
};

#endif
