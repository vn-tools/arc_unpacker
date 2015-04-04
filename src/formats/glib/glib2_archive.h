#ifndef FORMATS_GLIB_GLIB2_ARCHIVE_H
#define FORMATS_GLIB_GLIB2_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Glib
    {
        class Glib2Archive final : public Archive
        {
        public:
            Glib2Archive();
            ~Glib2Archive();
        protected:
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
