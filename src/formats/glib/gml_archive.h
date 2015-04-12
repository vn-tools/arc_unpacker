#ifndef FORMATS_GLIB_GML_ARCHIVE_H
#define FORMATS_GLIB_GML_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Glib
    {
        class GmlArchive final : public Archive
        {
        public:
            GmlArchive();
            ~GmlArchive();
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
