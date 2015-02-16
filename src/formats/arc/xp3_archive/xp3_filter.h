#ifndef FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER
#define FORMATS_ARC_XP3_ARCHIVE_XP3_FILTER
#include "file.h"

class Xp3Filter
{
public:
    virtual void decode(File &file, uint32_t key) const = 0;
};

#endif
