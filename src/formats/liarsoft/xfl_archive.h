#ifndef FORMATS_LIARSOFT_XFL_ARCHIVE_H
#define FORMATS_LIARSOFT_XFL_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace LiarSoft
    {
        class XflArchive final : public Archive
        {
        public:
            XflArchive();
            ~XflArchive();
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
