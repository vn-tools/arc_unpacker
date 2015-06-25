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
