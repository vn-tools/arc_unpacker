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
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Priv;
            std::unique_ptr<Priv> p;
        };
    }
}

#endif
