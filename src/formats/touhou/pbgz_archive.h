#ifndef FORMATS_TOUHOU_PBGZ_ARCHIVE_H
#define FORMATS_TOUHOU_PBGZ_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class PbgzArchive final : public Archive
        {
        public:
            PbgzArchive();
            ~PbgzArchive();
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
