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
