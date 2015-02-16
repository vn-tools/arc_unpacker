#ifndef FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER_FSN
#define FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER_FSN
#include "formats/arc/xp3_archive/xp3_filter.h"

class Xp3FilterFsn final : public Xp3Filter
{
public:
    virtual void decode(File &file, uint32_t key) const override;
};

#endif
