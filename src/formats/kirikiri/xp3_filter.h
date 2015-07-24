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
            virtual void set_arc_path(const std::string &path);
        protected:
            std::string arc_path;
        };
    }
}

#endif
