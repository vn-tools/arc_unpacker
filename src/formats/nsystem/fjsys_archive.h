#ifndef FORMATS_NSYSTEM_FJSYS_ARCHIVE_H
#define FORMATS_NSYSTEM_FJSYS_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace NSystem
    {
        class FjsysArchive final : public Archive
        {
        public:
            FjsysArchive();
            ~FjsysArchive();
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
