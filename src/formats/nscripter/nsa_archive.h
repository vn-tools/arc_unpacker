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
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Priv;
            std::unique_ptr<Priv> p;
        };
    }
}

#endif
