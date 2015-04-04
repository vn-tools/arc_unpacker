#ifndef FORMATS_NSCRIPTER_NSA_ARCHIVE_H
#define FORMATS_NSCRIPTER_NSA_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace NScripter
    {
        class NsaArchive final : public Archive
        {
        public:
            NsaArchive();
            ~NsaArchive();
        protected:
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
