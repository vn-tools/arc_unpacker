#ifndef FORMATS_KIRIKIRI_XP3_FILTER_H
#define FORMATS_KIRIKIRI_XP3_FILTER_H
#include "file.h"

namespace Formats
{
    namespace Kirikiri
    {
        class Xp3Filter
        {
        public:
            virtual void decode(File &file, u32 key) const = 0;
        };
    }
}

#endif
