#ifndef FORMATS_LIARSOFT_LWG_ARCHIVE_H
#define FORMATS_LIARSOFT_LWG_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace LiarSoft
    {
        class LwgArchive final : public Archive
        {
        public:
            LwgArchive();
            ~LwgArchive();
        protected:
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
