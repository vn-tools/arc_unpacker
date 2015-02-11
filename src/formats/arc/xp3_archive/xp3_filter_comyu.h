#ifndef FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER_COMYU
#define FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER_COMYU
#include "formats/arc/xp3_archive/xp3_filter.h"

class Xp3FilterComyu final : public Xp3Filter
{
public:
    virtual void decode(VirtualFile &file, uint32_t key) const override;
};

#endif
