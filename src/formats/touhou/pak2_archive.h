#ifndef FORMATS_TOUHOU_PAK2_ARCHIVE_H
#define FORMATS_TOUHOU_PAK2_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class Pak2Archive final : public Archive
        {
        public:
            Pak2Archive();
            ~Pak2Archive();
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
