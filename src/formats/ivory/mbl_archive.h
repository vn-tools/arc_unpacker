#ifndef FORMATS_IVORY_MBL_ARCHIVE_H
#define FORMATS_IVORY_MBL_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Ivory
    {
        class MblArchive final : public Archive
        {
        public:
            MblArchive();
            ~MblArchive();
        protected:
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
